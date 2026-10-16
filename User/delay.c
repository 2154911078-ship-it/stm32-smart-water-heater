/*
 * delay.c  --  SysTick 定时
 *
 * SysTick 设为 1ms 中断, 在中断里累加毫秒计数;
 * 微秒延时用轮询 SysTick 的递减计数器实现, 精度不依赖编译器优化。
 */
#include "board.h"
#include "delay.h"

static volatile uint32_t s_ms = 0u;

void delay_init(void)
{
#if !DELAY_USE_LOOP
    SYSTICK->LOAD = (SYSCLK_HZ / 1000u) - 1u;       /* 1ms 重装值 */
    SYSTICK->VAL  = 0u;
    SYSTICK->CTRL = SYSTICK_CTRL_CLKSOURCE | SYSTICK_CTRL_TICKINT | SYSTICK_CTRL_ENABLE;
#endif
}

/* SysTick 中断服务函数: Keil 的 startup_stm32f10x_ld.s 与本工程的
 * startup_stm32f103c6.s 都有同名向量, 会自动使用这里的实现。          */
void SysTick_Handler(void)
{
    s_ms++;
}

uint32_t millis(void)
{
    return s_ms;
}

void delay_tick_disable(void)
{
    SYSTICK->CTRL &= ~SYSTICK_CTRL_TICKINT;
}

void delay_tick_enable(void)
{
    SYSTICK->CTRL |= SYSTICK_CTRL_TICKINT;
}

uint8_t time_reached(uint32_t deadline_ms)
{
    return (uint8_t)(((int32_t)(millis() - deadline_ms)) >= 0);
}

#if DELAY_USE_LOOP
/* 备用方案: 纯指令循环延时(万一仿真环境不模拟 SysTick 时改用)
 * 每微秒约 SYSCLK_HZ/1000000 个时钟周期, 循环一次按 4 周期估算。 */
void delay_us(uint32_t us)
{
    while (us != 0u) {
        volatile uint32_t n = (SYSCLK_HZ / 1000000u) / 4u;
        while (n != 0u) {
            n--;
        }
        us--;
    }
}
#else
void delay_us(uint32_t us)
{
    uint32_t ticks = us * (SYSCLK_HZ / 1000000u);
    uint32_t start;

    if (ticks == 0u) {
        ticks = 1u;
    }
    start = SYSTICK->VAL & 0xFFFFFFu;
    while (((start - (SYSTICK->VAL & 0xFFFFFFu)) & 0xFFFFFFu) < ticks) {
        /* 空转等待 */
    }
}
#endif

void delay_ms(uint32_t ms)
{
    while (ms != 0u) {
        delay_us(1000u);
        ms--;
    }
}
