/*
 * lcd1602.h  --  LCD1602 驱动(8 位并口)
 *   D0~D7 : PA0~PA7
 *   RS    : PA8
 *   RW    : PA9   (始终写模式, 不读忙标志, 用固定延时等待)
 *   E     : PA10
 */
#ifndef LCD1602_H
#define LCD1602_H

#include <stdint.h>

void lcd_init(void);
void lcd_cmd(uint8_t cmd);
void lcd_data(uint8_t dat);
void lcd_clear(void);
void lcd_set_cursor(uint8_t row, uint8_t col);
void lcd_puts(const char *s);
/* 在指定位置连续写 len 个字符(不足补空格), 用于整行刷新, 不闪烁 */
void lcd_puts_at(uint8_t row, uint8_t col, const char *s, uint8_t len);

#endif /* LCD1602_H */
