/*
 * lcd1602.c  --  LCD1602 8 位驱动(HD44780 兼容)
 */
#include "board.h"
#include "bsp.h"
#include "delay.h"
#include "lcd1602.h"

/* 产生一个 E 使能脉冲(数据在 E 下降沿被锁存) */
static void lcd_pulse_e(void)
{
    PIN_SET(LCD_E_PORT, LCD_E_PIN);
    delay_us(3);
    PIN_CLR(LCD_E_PORT, LCD_E_PIN);
    delay_us(60);                   /* 等待控制器内部处理(>40us) */
}

void lcd_cmd(uint8_t cmd)
{
    PIN_CLR(LCD_RS_PORT, LCD_RS_PIN);   /* RS = 0 : 指令 */
    PIN_CLR(LCD_RW_PORT, LCD_RW_PIN);   /* RW = 0 : 写   */
    LCD_DATA_WRITE(cmd);
    lcd_pulse_e();
}

void lcd_data(uint8_t dat)
{
    PIN_SET(LCD_RS_PORT, LCD_RS_PIN);   /* RS = 1 : 数据 */
    PIN_CLR(LCD_RW_PORT, LCD_RW_PIN);
    LCD_DATA_WRITE(dat);
    lcd_pulse_e();
}

void lcd_clear(void)
{
    lcd_cmd(0x01);
    delay_ms(3);
}

void lcd_set_cursor(uint8_t row, uint8_t col)
{
    uint8_t addr = (uint8_t)((row == 0u) ? 0x00u : 0x40u);

    lcd_cmd((uint8_t)(0x80u | (uint8_t)(addr + col)));
}

void lcd_puts(const char *s)
{
    while (*s != '\0') {
        lcd_data((uint8_t)(*s));
        s++;
    }
}

void lcd_puts_at(uint8_t row, uint8_t col, const char *s, uint8_t len)
{
    uint8_t i;

    lcd_set_cursor(row, col);
    for (i = 0u; i < len; i++) {
        char c = s[i];
        lcd_data((uint8_t)((c == '\0') ? ' ' : c));
    }
}

void lcd_init(void)
{
    uint8_t i;

    /* 1. 数据线 D0..D7 (数据口从第 LCD_DATA_SHIFT 位开始的 8 个脚),
     *    以及 RS / RW / E */
    for (i = 0u; i < 8u; i++) {
        bsp_gpio_output(LCD_DATA_PORT, (uint8_t)(i + LCD_DATA_SHIFT));
    }
    bsp_gpio_output(LCD_RS_PORT, LCD_RS_PIN);
    bsp_gpio_output(LCD_RW_PORT, LCD_RW_PIN);
    bsp_gpio_output(LCD_E_PORT,  LCD_E_PIN);

    PIN_CLR(LCD_E_PORT,  LCD_E_PIN);
    PIN_CLR(LCD_RS_PORT, LCD_RS_PIN);
    PIN_CLR(LCD_RW_PORT, LCD_RW_PIN);
    LCD_DATA_WRITE(0x00u);

    /* 2. 上电初始化时序(数据手册 Figure 24) */
    delay_ms(50);
    lcd_cmd(0x38);                  /* 8 位, 2 行, 5x8 点阵 */
    delay_ms(5);
    lcd_cmd(0x38);
    delay_us(200);
    lcd_cmd(0x38);
    delay_us(200);

    lcd_cmd(0x0C);                  /* 开显示, 关光标, 不闪烁 */
    lcd_cmd(0x06);                  /* 写入后光标右移, 屏幕不移动 */
    lcd_clear();
}
