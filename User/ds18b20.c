/*
 * ds18b20.c  --  DS18B20 1-Wire 驱动
 *
 * 引脚 PA11 配置为"开漏输出": 写 0 拉低总线, 写 1 释放总线(由 10K 上拉电阻
 * 拉高), 同时 IDR 一直可以读回总线电平 —— 这是最省事的单总线实现方式。
 *
 * 时序(标准速度)依据 DS18B20 数据手册:
 *   复位       : 拉低 >480us -> 释放 -> 15~60us 内采样存在脉冲
 *   写 1 时隙  : 拉低 2us -> 释放 -> 共 60us
 *   写 0 时隙  : 拉低 60us -> 释放 2us
 *   读 时隙    : 拉低 2us -> 释放 -> 10us 后采样 -> 共 60us
 */
#include "board.h"
#include "bsp.h"
#include "delay.h"
#include "ds18b20.h"

#define OW_LOW()        ow_drive_low()
#define OW_RELEASE()    ow_release_bus()
#define OW_LEVEL()      ((uint8_t)PIN_READ(OW_PORT, OW_PIN))

/* 拉低总线 */
static void ow_drive_low(void)
{
#if OW_MODE_SWITCH
    bsp_gpio_config(OW_PORT, OW_PIN, GPIO_MODE_OUT_50M, GPIO_CNF_OUT_PP);
#endif
    PIN_CLR(OW_PORT, OW_PIN);
}

/* 释放总线(开漏输出写 1 / 或切成上拉输入) */
static void ow_release_bus(void)
{
#if OW_MODE_SWITCH
    bsp_gpio_config(OW_PORT, OW_PIN, GPIO_MODE_INPUT, GPIO_CNF_IN_PUPD);
    PIN_SET(OW_PORT, OW_PIN);           /* 打开内部上拉 */
#else
    PIN_SET(OW_PORT, OW_PIN);
#endif
}

#define DS18B20_CMD_SKIP_ROM    0xCCu
#define DS18B20_CMD_CONVERT_T   0x44u
#define DS18B20_CMD_READ_SCR    0xBEu

void ds18b20_init(void)
{
#if OW_MODE_SWITCH
    ow_release_bus();               /* 切成上拉输入, 总线保持高电平 */
#else
    bsp_gpio_od(OW_PORT, OW_PIN);   /* 开漏输出, 写 1 即释放总线 */
#endif
}

static uint8_t ow_reset(void)
{
    uint8_t presence;

    OW_LOW();
    delay_us(480);
    OW_RELEASE();
    delay_us(70);
    presence = OW_LEVEL();          /* 0 = 器件拉低总线, 表示存在 */
    delay_us(410);
    return (uint8_t)((presence == 0u) ? 1u : 0u);
}

static void ow_write_byte(uint8_t b)
{
    uint8_t i;

    for (i = 0u; i < 8u; i++) {
        OW_LOW();
        if ((b & 0x01u) != 0u) {
            delay_us(2);
            OW_RELEASE();
            delay_us(60);
        } else {
            delay_us(60);
            OW_RELEASE();
            delay_us(2);
        }
        b = (uint8_t)(b >> 1);
    }
}

static uint8_t ow_read_byte(void)
{
    uint8_t i;
    uint8_t b = 0u;

    for (i = 0u; i < 8u; i++) {
        OW_LOW();
        delay_us(2);
        OW_RELEASE();
        delay_us(10);
        if (OW_LEVEL() != 0u) {
            b = (uint8_t)(b | (uint8_t)(1u << i));
        }
        delay_us(50);
    }
    return b;
}

uint8_t ds18b20_start_convert(void)
{
    uint8_t ok;

    delay_tick_disable();           /* 1-Wire 期间不要被 1ms 中断打断 */
    ok = ow_reset();
    if (ok != 0u) {
        ow_write_byte(DS18B20_CMD_SKIP_ROM);    /* 只挂一个传感器, 跳过 ROM */
        ow_write_byte(DS18B20_CMD_CONVERT_T);
    }
    delay_tick_enable();
    return ok;
}

uint8_t ds18b20_read_raw(int16_t *raw)
{
    uint8_t ok;
    uint8_t lo;
    uint8_t hi;

    delay_tick_disable();
    ok = ow_reset();
    if (ok != 0u) {
        ow_write_byte(DS18B20_CMD_SKIP_ROM);
        ow_write_byte(DS18B20_CMD_READ_SCR);
        lo = ow_read_byte();
        hi = ow_read_byte();
        *raw = (int16_t)(((uint16_t)hi << 8) | (uint16_t)lo);
    }
    delay_tick_enable();
    return ok;
}

int16_t ds18b20_raw_to_tenths(int16_t raw)
{
    int32_t t = (int32_t)raw;

    if (t >= 0) {
        return (int16_t)((t * 10 + 8) / 16);        /* 四舍五入到 0.1℃ */
    }
    return (int16_t)((t * 10 - 8) / 16);
}
