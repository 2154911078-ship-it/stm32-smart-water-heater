/*
 * bsp.h  --  板级支持: 时钟 / GPIO / 输出(继电器+指示灯) / 输入(按键+缺水开关)
 */
#ifndef BSP_H
#define BSP_H

#include <stdint.h>
#include "stm32f1_regs.h"

/* 系统时钟: 打开 HSE(8MHz), 按 board.h 的 USE_PLL_72MHZ 决定是否倍频 */
void bsp_clock_init(void);

/* GPIO 配置 */
void bsp_gpio_config(GPIO_TypeDef *port, uint8_t pin, uint8_t mode, uint8_t cnf);
void bsp_gpio_output(GPIO_TypeDef *port, uint8_t pin);          /* 推挽输出 */
void bsp_gpio_od(GPIO_TypeDef *port, uint8_t pin);              /* 开漏输出 */
void bsp_gpio_input(GPIO_TypeDef *port, uint8_t pin, uint8_t pullup); /* 上下拉输入 */

/* 输出: 继电器 + 缺水指示灯 */
void bsp_outputs_init(void);
void bsp_outputs_apply(uint8_t heating, uint8_t filling, uint8_t shortage);

/* 输入 */
void    bsp_inputs_init(void);
uint8_t bsp_keys_read(void);        /* bit0=L+ bit1=L- bit2=H+ bit3=H-, 1=按下 */
uint8_t bsp_sensor_short(void);     /* 1 = 缺水 */

#endif /* BSP_H */
