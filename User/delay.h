/*
 * delay.h  --  SysTick 毫秒计时 + 微秒/毫秒延时
 */
#ifndef DELAY_H
#define DELAY_H

#include <stdint.h>

void     delay_init(void);
void     delay_us(uint32_t us);
void     delay_ms(uint32_t ms);
uint32_t millis(void);                      /* 开机以来的毫秒数 */

/* 1-Wire 等对时序敏感的场合: 暂时关掉 1ms 中断, 结束后再打开 */
void     delay_tick_disable(void);
void     delay_tick_enable(void);

/* 到点判断(支持 millis 溢出) */
uint8_t  time_reached(uint32_t deadline_ms);

#endif /* DELAY_H */
