/*
 * stm32f1_regs.h  --  最小化 STM32F1 寄存器定义
 *
 * 只包含本工程用到的寄存器，不依赖 CMSIS / 标准外设库，
 * 因此 Keil(ARMCC5 / ARMCLANG6) 与 arm-none-eabi-gcc 都能直接编译。
 * 用户原来的 STM32F103R6 工程里的 stm32f10x.h / 库文件都可以留着不用，
 * 只把 main 和驱动换成本工程的源文件即可。
 */
#ifndef STM32F1_REGS_H
#define STM32F1_REGS_H

#include <stdint.h>

typedef struct {
    volatile uint32_t CRL;      /* 0x00 端口配置低寄存器  (pin 0..7)  */
    volatile uint32_t CRH;      /* 0x04 端口配置高寄存器  (pin 8..15) */
    volatile uint32_t IDR;      /* 0x08 输入数据寄存器                */
    volatile uint32_t ODR;      /* 0x0C 输出数据寄存器                */
    volatile uint32_t BSRR;     /* 0x10 位设置/清除寄存器             */
    volatile uint32_t BRR;      /* 0x14 位清除寄存器                  */
    volatile uint32_t LCKR;     /* 0x18 配置锁定寄存器                */
} GPIO_TypeDef;

typedef struct {
    volatile uint32_t CR;       /* 0x00 时钟控制寄存器                */
    volatile uint32_t CFGR;     /* 0x04 时钟配置寄存器                */
    volatile uint32_t CIR;      /* 0x08 时钟中断寄存器                */
    volatile uint32_t APB2RSTR; /* 0x0C                              */
    volatile uint32_t APB1RSTR; /* 0x10                              */
    volatile uint32_t AHBENR;   /* 0x14                              */
    volatile uint32_t APB2ENR;  /* 0x18 APB2 外设时钟使能             */
    volatile uint32_t APB1ENR;  /* 0x1C APB1 外设时钟使能             */
    volatile uint32_t BDCR;     /* 0x20 备份域控制寄存器              */
    volatile uint32_t CSR;      /* 0x24 控制/状态寄存器               */
} RCC_TypeDef;

typedef struct {
    volatile uint32_t ACR;      /* 0x00 访问控制寄存器(等待周期)      */
    volatile uint32_t KEYR;
    volatile uint32_t OPTKEYR;
    volatile uint32_t SR;
    volatile uint32_t CR;
    volatile uint32_t AR;
    volatile uint32_t RESERVED;
    volatile uint32_t OBR;
    volatile uint32_t WRPR;
} FLASH_TypeDef;

typedef struct {
    volatile uint32_t CTRL;     /* 0x00 */
    volatile uint32_t LOAD;     /* 0x04 */
    volatile uint32_t VAL;      /* 0x08 */
    volatile uint32_t CALIB;    /* 0x0C */
} SysTick_TypeDef;

/* ---------------- 总线 / 外设基地址 ---------------- */
#define PERIPH_BASE      0x40000000u
#define APB1PERIPH_BASE  (PERIPH_BASE + 0x00000000u)
#define APB2PERIPH_BASE  (PERIPH_BASE + 0x00010000u)
#define AHBPERIPH_BASE   (PERIPH_BASE + 0x00020000u)

#define GPIOA   ((GPIO_TypeDef  *)(APB2PERIPH_BASE + 0x0800u))   /* 0x40010800 */
#define GPIOB   ((GPIO_TypeDef  *)(APB2PERIPH_BASE + 0x0C00u))   /* 0x40010C00 */
#define GPIOC   ((GPIO_TypeDef  *)(APB2PERIPH_BASE + 0x1000u))   /* 0x40011000 */
#define RCC     ((RCC_TypeDef   *)(AHBPERIPH_BASE  + 0x1000u))   /* 0x40021000 */
#define FLASH   ((FLASH_TypeDef *)(AHBPERIPH_BASE  + 0x2000u))   /* 0x40022000 */
#define SYSTICK ((SysTick_TypeDef *)0xE000E010u)

/* ---------------- 位定义 ---------------- */
/* RCC_CR */
#define RCC_CR_HSION        (1u << 0)
#define RCC_CR_HSIRDY       (1u << 1)
#define RCC_CR_HSEON        (1u << 16)
#define RCC_CR_HSERDY       (1u << 17)
#define RCC_CR_PLLON        (1u << 24)
#define RCC_CR_PLLRDY       (1u << 25)

/* RCC_CFGR : SW[1:0] SWS[3:2] HPRE[7:4] PPRE1[10:8] PPRE2[13:11] PLLSRC[16] PLLXTPRE[17] PLLMUL[21:18] */
#define RCC_CFGR_SW_HSI     0x0u
#define RCC_CFGR_SW_HSE     0x1u
#define RCC_CFGR_SW_PLL     0x2u

/* RCC_APB2ENR */
#define RCC_APB2ENR_AFIOEN  (1u << 0)
#define RCC_APB2ENR_IOPAEN  (1u << 2)
#define RCC_APB2ENR_IOPBEN  (1u << 3)
#define RCC_APB2ENR_IOPCEN  (1u << 4)

/* SysTick */
#define SYSTICK_CTRL_ENABLE     (1u << 0)
#define SYSTICK_CTRL_TICKINT    (1u << 1)
#define SYSTICK_CTRL_CLKSOURCE  (1u << 2)
#define SYSTICK_CTRL_COUNTFLAG  (1u << 16)

/* GPIO 配置字: CNF[1:0] << 2 | MODE[1:0] */
#define GPIO_MODE_INPUT        0x0u    /* 输入                       */
#define GPIO_MODE_OUT_10M      0x1u
#define GPIO_MODE_OUT_2M       0x2u
#define GPIO_MODE_OUT_50M      0x3u
#define GPIO_CNF_IN_ANALOG     0x0u
#define GPIO_CNF_IN_FLOAT      0x1u
#define GPIO_CNF_IN_PUPD       0x2u    /* 上下拉输入(由 ODR 选择)    */
#define GPIO_CNF_OUT_PP        0x0u    /* 推挽输出                   */
#define GPIO_CNF_OUT_OD        0x1u    /* 开漏输出                   */
#define GPIO_CNF_AF_PP         0x2u

/* 位操作宏 */
#define PIN_SET(port, pin)          ((port)->BSRR = (1u << (pin)))
#define PIN_CLR(port, pin)          ((port)->BSRR = (1u << ((pin) + 16)))
#define PIN_WRITE(port, pin, v)     ((port)->BSRR = ((v) ? (1u << (pin)) : (1u << ((pin) + 16))))
#define PIN_READ(port, pin)         (((port)->IDR >> (pin)) & 0x1u)
#define PIN_TOGGLE(port, pin)       ((port)->ODR ^= (1u << (pin)))

#endif /* STM32F1_REGS_H */
