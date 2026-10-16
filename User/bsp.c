/*
 * bsp.c  --  时钟初始化 + GPIO + 输出/输入
 * 目标: STM32F103C6T6 (LQFP48)
 */
#include "bsp.h"
#include "board.h"
#include "control.h"        /* 只为 KEY_x 序号与 KEY_COUNT 的一致定义 */

/* ------------------------------------------------------------------ */
/*  时钟:  默认 HSE 8MHz 直接作为 SYSCLK(与 Proteus 中 U1 的            */
/*         Crystal Frequency 属性 8MHz 一致, 时序最稳)                  */
/*         USE_PLL_72MHZ=1 时改为 HSE x9 = 72MHz                       */
/* ------------------------------------------------------------------ */
void bsp_clock_init(void)
{
    uint32_t timeout;

    /* 1. 打开外部 8MHz 晶振 */
    RCC->CR |= RCC_CR_HSEON;
    timeout = 0x20000u;
    while ((!(RCC->CR & RCC_CR_HSERDY)) && (timeout != 0u)) {
        timeout--;
    }

    if (timeout == 0u) {
        /* 外部晶振起不来(仿真里没画晶振也不会死机): 退回内部 HSI 8MHz */
        RCC->CFGR = RCC_CFGR_SW_HSI;
        while (((RCC->CFGR >> 2) & 0x3u) != RCC_CFGR_SW_HSI) {
        }
        RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;
        return;
    }

    /* 2. 先切回 HSI(8MHz) 再改 FLASH 等待周期和 PLL:
     *    如果原来的 Keil 工程里 system_stm32f10x.c 已经把主频设成 72MHz,
     *    这里必须先把频率降下来, 否则会出现"高频 + 少等待周期"的危险组合 */
    RCC->CR &= ~RCC_CR_PLLON;
    RCC->CFGR = RCC_CFGR_SW_HSI;
    while (((RCC->CFGR >> 2) & 0x3u) != RCC_CFGR_SW_HSI) {
    }

    /* 3. FLASH 等待周期 */
#if USE_PLL_72MHZ
    FLASH->ACR = 0x12u;             /* 2 个等待周期 + 预取使能 */
#else
    FLASH->ACR = 0x10u;             /* 0 个等待周期 + 预取使能 */
#endif

#if USE_PLL_72MHZ
    RCC->CFGR = (0x0u << 4)         /* HPRE  = /1  -> AHB  72MHz */
              | (0x4u << 8)         /* PPRE1 = /2  -> APB1 36MHz */
              | (0x0u << 11)        /* PPRE2 = /1  -> APB2 72MHz */
              | (1u << 16)          /* PLLSRC = HSE              */
              | (7u << 18);         /* PLLMUL = x9               */
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY)) {
    }
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while (((RCC->CFGR >> 2) & 0x3u) != RCC_CFGR_SW_PLL) {
    }
#else
    RCC->CFGR = 0x0u;               /* AHB/APB 均不分频            */
    RCC->CFGR |= RCC_CFGR_SW_HSE;
    while (((RCC->CFGR >> 2) & 0x3u) != RCC_CFGR_SW_HSE) {
    }
#endif

    /* 4. GPIO 时钟 (PA / PB / AFIO) */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;
}

/* ------------------------------------------------------------------ */
/*  GPIO                                                              */
/* ------------------------------------------------------------------ */
void bsp_gpio_config(GPIO_TypeDef *port, uint8_t pin, uint8_t mode, uint8_t cnf)
{
    volatile uint32_t *cr;
    uint32_t shift;
    uint32_t cfg;

    cr    = (pin < 8u) ? &port->CRL : &port->CRH;
    shift = (uint32_t)(pin & 0x7u) * 4u;
    cfg   = ((uint32_t)cnf << 2) | (uint32_t)mode;

    *cr = (*cr & ~(0xFu << shift)) | (cfg << shift);
}

void bsp_gpio_output(GPIO_TypeDef *port, uint8_t pin)
{
    bsp_gpio_config(port, pin, GPIO_MODE_OUT_50M, GPIO_CNF_OUT_PP);
}

void bsp_gpio_od(GPIO_TypeDef *port, uint8_t pin)
{
    bsp_gpio_config(port, pin, GPIO_MODE_OUT_50M, GPIO_CNF_OUT_OD);
    PIN_SET(port, pin);             /* 开漏输出默认释放总线(高阻) */
}

void bsp_gpio_input(GPIO_TypeDef *port, uint8_t pin, uint8_t pullup)
{
    bsp_gpio_config(port, pin, GPIO_MODE_INPUT, GPIO_CNF_IN_PUPD);
    PIN_WRITE(port, pin, pullup);
}

/* ------------------------------------------------------------------ */
/*  输出: 加热继电器 / 加水继电器 / 缺水指示灯                          */
/* ------------------------------------------------------------------ */
void bsp_outputs_init(void)
{
    bsp_gpio_output(RELAY_HEAT_PORT, RELAY_HEAT_PIN);
    bsp_gpio_output(RELAY_FILL_PORT, RELAY_FILL_PIN);
    bsp_gpio_output(LED_SHORT_PORT,  LED_SHORT_PIN);

#if RELAY_ACTIVE_HIGH
    PIN_CLR(RELAY_HEAT_PORT, RELAY_HEAT_PIN);
    PIN_CLR(RELAY_FILL_PORT, RELAY_FILL_PIN);
#else
    PIN_SET(RELAY_HEAT_PORT, RELAY_HEAT_PIN);
    PIN_SET(RELAY_FILL_PORT, RELAY_FILL_PIN);
#endif

#if LED_SHORT_ACTIVE_LOW
    PIN_SET(LED_SHORT_PORT, LED_SHORT_PIN);     /* 灭 */
#else
    PIN_CLR(LED_SHORT_PORT, LED_SHORT_PIN);
#endif
}

void bsp_outputs_apply(uint8_t heating, uint8_t filling, uint8_t shortage)
{
#if RELAY_ACTIVE_HIGH
    PIN_WRITE(RELAY_HEAT_PORT, RELAY_HEAT_PIN, heating);
    PIN_WRITE(RELAY_FILL_PORT, RELAY_FILL_PIN, filling);
#else
    PIN_WRITE(RELAY_HEAT_PORT, RELAY_HEAT_PIN, (uint8_t)!heating);
    PIN_WRITE(RELAY_FILL_PORT, RELAY_FILL_PIN, (uint8_t)!filling);
#endif

#if LED_SHORT_ACTIVE_LOW
    PIN_WRITE(LED_SHORT_PORT, LED_SHORT_PIN, (uint8_t)!shortage);
#else
    PIN_WRITE(LED_SHORT_PORT, LED_SHORT_PIN, shortage);
#endif
}

/* ------------------------------------------------------------------ */
/*  输入: 4 个按键 + 缺水感应开关                                       */
/* ------------------------------------------------------------------ */
void bsp_inputs_init(void)
{
    uint8_t i;

    /* 注意: 上拉/下拉方向与"有效电平"相反 ---- 
     * 按键接 GND(低有效) -> 内部上拉;  按键接 +3.3V(高有效) -> 内部下拉 */
    for (i = 0u; i < (uint8_t)KEY_COUNT; i++) {
        bsp_gpio_input(KEY_PORT, (uint8_t)(KEY_FIRST_PIN + i),
                       (uint8_t)((KEY_ACTIVE_HIGH) ? 0u : 1u));
    }
    bsp_gpio_input(SENSOR_SHORT_PORT, SENSOR_SHORT_PIN,
                   (uint8_t)((SENSOR_ACTIVE_HIGH) ? 0u : 1u));
}

uint8_t bsp_keys_read(void)
{
    uint8_t raw = (uint8_t)((KEY_PORT->IDR >> KEY_FIRST_PIN) & 0x0Fu);

#if KEY_ACTIVE_HIGH
    return raw;                             /* 高电平 = 按下 */
#else
    return (uint8_t)((~raw) & 0x0Fu);       /* 低电平 = 按下 */
#endif
}

uint8_t bsp_sensor_short(void)
{
    uint8_t v = (uint8_t)PIN_READ(SENSOR_SHORT_PORT, SENSOR_SHORT_PIN);

#if SENSOR_ACTIVE_HIGH
    return v;                               /* 高电平 = 缺水 */
#else
    return (uint8_t)(!v);
#endif
}
