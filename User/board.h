/*
 * board.h  --  引脚分配与功能开关（换接线只需要改这个文件）
 *
 * 目标芯片: STM32F103C6T6  (LQFP48, 32KB Flash, 10KB RAM)
 *
 * 设计原则: 让改动量最小 ---- 原来接在 PB/PA 上的液晶和 DS18B20 一根线都不用动,
 *           只有原来接在 PC 口的 8 根信号线要挪到 PA 口。
 *
 * ------------------------------------------------------------------
 *  功能              STM32 引脚   LQFP48 脚号    和原 R6 原理图相比
 * ------------------------------------------------------------------
 *  LCD1602 D0        PB8          45            不变
 *  LCD1602 D1        PB9          46            不变
 *  LCD1602 D2        PB10         21            不变
 *  LCD1602 D3        PB11         22            不变
 *  LCD1602 D4        PB12         25            不变
 *  LCD1602 D5        PB13         26            不变
 *  LCD1602 D6        PB14         27            不变
 *  LCD1602 D7        PB15         28            不变
 *  LCD1602 RS        PB5          41            不变
 *  LCD1602 RW        PB6          42            不变
 *  LCD1602 E         PB7          43            不变
 *  DS18B20 DQ        PA8          29            不变（10K 上拉到 +3.3V）
 *  温度下限 +  (L+)  PA0          10            原来 PC0
 *  温度下限 -  (L-)  PA1          11            原来 PC1
 *  温度上限 +  (H+)  PA2          12            原来 PC2
 *  温度上限 -  (H-)  PA3          13            原来 PC3
 *  缺水感应开关      PA4          14            原来 PC4
 *  缺水指示灯 D5     PA5          15            原来 PC 口
 *  加热继电器 Q1     PA6          16            原来 PC 口
 *  加水继电器 Q2     PA7          17            原来 PC 口
 * ------------------------------------------------------------------
 *  说明: PA13 / PA14 留作 SWD 下载, PB3 / PB4 / PA15 留作 JTAG,
 *        PB2(BOOT1) 也没有使用, 全部空着。
 */
#ifndef BOARD_H
#define BOARD_H

#include "stm32f1_regs.h"

/* ==================== 系统时钟 ==================== */
/* Proteus 中 U1(STM32F103C6) 的 "Crystal Frequency" 属性请填 8MHz  */
#define HSE_HZ              8000000u
#define USE_PLL_72MHZ       0           /* 1 = HSE 8MHz x9 = 72MHz */
#if USE_PLL_72MHZ
#define SYSCLK_HZ           72000000u
#else
#define SYSCLK_HZ           HSE_HZ
#endif

/* 1 = 用指令循环做微秒延时(万一仿真里 SysTick 不计数时可切换)   */
#define DELAY_USE_LOOP      0

/* ==================== LCD1602 ==================== */
/* 数据线 D0..D7 接在 PB8..PB15 (即整个端口的高 8 位)              */
#define LCD_DATA_PORT       GPIOB
#define LCD_DATA_SHIFT      8
/* RS / RW / E 接在 PB5 / PB6 / PB7                                */
#define LCD_RS_PORT         GPIOB
#define LCD_RS_PIN          5
#define LCD_RW_PORT         GPIOB
#define LCD_RW_PIN          6
#define LCD_E_PORT          GPIOB
#define LCD_E_PIN           7

/* 一次把 8 位数据写到数据口(用 BSRR 一次完成, 不影响其它引脚)。
 * 只用 16 位寄存器的一半: 低半区置 1, 高半区清 0, 所以 PB5/PB6/PB7 不受影响。 */
#define LCD_DATA_WRITE(v)   do {                                                        \
        (LCD_DATA_PORT)->BSRR = (((uint32_t)((~(v)) & 0xFFu) << (16u + LCD_DATA_SHIFT))  \
                              |  ((uint32_t)((v) & 0xFFu) << LCD_DATA_SHIFT));          \
    } while (0)

/* ==================== DS18B20 ==================== */
#define OW_PORT             GPIOA
#define OW_PIN              8

/* 单总线引脚的处理方式:
 *   0 = 引脚固定为"开漏输出", 写 1 释放总线由 10K 电阻拉高, 直接读 IDR(默认)
 *   1 = 读写时在"推挽输出 / 上拉输入"之间切换(经典 51 写法, 兼容性更好)
 * 如果仿真里温度始终读不到, 可以把这个宏改成 1 重新编译             */
#ifndef OW_MODE_SWITCH
#define OW_MODE_SWITCH      0
#endif

/* ==================== 继电器 / 指示灯 ==================== */
#define RELAY_HEAT_PORT     GPIOA       /* 加热继电器: Q1(TIP122) 基极经 1K */
#define RELAY_HEAT_PIN      6
#define RELAY_FILL_PORT     GPIOA       /* 加水继电器: Q2(TIP122) 基极经 1K */
#define RELAY_FILL_PIN      7
#define RELAY_ACTIVE_HIGH   1           /* 1 = 输出高电平吸合 */

#define LED_SHORT_PORT      GPIOA       /* 缺水指示灯 D5(阴极经 100Ω 接 IO) */
#define LED_SHORT_PIN       5
#define LED_SHORT_ACTIVE_LOW 1          /* 1 = 输出低电平点亮 */

/* ==================== 输入 ==================== */
#define SENSOR_SHORT_PORT   GPIOA       /* 缺水感应开关 */
#define SENSOR_SHORT_PIN    4
#define SENSOR_ACTIVE_HIGH  1           /* 1 = 开关闭合时输入高电平 = 缺水 */

#define KEY_PORT            GPIOA       /* 四个设置按键, 连续 4 位便于一次读入 */
#define KEY_L_PLUS_PIN      0
#define KEY_L_MINUS_PIN     1
#define KEY_H_PLUS_PIN      2
#define KEY_H_MINUS_PIN     3
#define KEY_FIRST_PIN       0
/* 0 = 按键一端接 GND, 内部上拉, 按下为低电平(默认)
 * 1 = 按键一端接 +3.3V, 内部下拉, 按下为高电平
 * 原设计(带 RP1 排阻那份)按键公共端接的是 +3.3V, 如果照原样接就用 1,
 * 工程里已经附带编好的 build/heat_c6_btnhigh.hex                    */
#ifndef KEY_ACTIVE_HIGH
#define KEY_ACTIVE_HIGH     0
#endif

/* ==================== 运行参数 ==================== */
#define SAMPLE_PERIOD_MS    800         /* DS18B20 采样周期(转换需要 750ms)  */
/* 按键扫描周期: 默认 20ms(要连续两次一致才认账, 抗抖动)。
 * 仿真里如果"按一下没反应", 可以把扫描周期缩短到 5ms 重新编译       */
#ifndef KEY_SCAN_MS
#define KEY_SCAN_MS         20
#endif
#define KEY_REPEAT_DELAY_MS 500         /* 长按开始连发的延时                */
#define KEY_REPEAT_MS       150         /* 连发间隔                          */

/* 缺水时是否禁止加热: 1 = 互不影响(按题目要求), 0 = 缺水时强制停止加热 */
#define ALLOW_HEAT_WHEN_SHORT   1

#endif /* BOARD_H */
