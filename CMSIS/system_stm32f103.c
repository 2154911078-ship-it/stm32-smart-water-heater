/*
 * system_stm32f103.c  --  最小系统初始化文件（Keil 工程用）
 *
 * 启动文件在调用 main 之前会先调用 SystemInit()。
 * 本工程的主频配置放在 bsp.c 的 bsp_clock_init() 里完成
 * （默认 HSE 8MHz 直接作为系统时钟，并且会先把时钟切回内部 HSI
 *   再重新配置，避免出现"高频 + 少等待周期"的危险组合），
 * 所以这里不需要做任何事情。
 *
 * 也正因为如此，这个 Keil 工程完全不依赖 ST 标准外设库
 * （不需要 stm32f10x.h、system_stm32f10x.c、startup_stm32f10x_ld.s）。
 */
#include <stdint.h>

/* 系统时钟频率(Hz)，与 board.h 里的 SYSCLK_HZ 保持一致 */
uint32_t SystemCoreClock = 8000000u;

void SystemInit(void)
{
    /* 空实现：时钟在 bsp_clock_init() 里配置 */
}
