/*
 * main.c  --  基于 STM32F103C6T6 的电热水器控制器
 *
 * 功能:
 *   (1) LCD1602 显示当前水温、水温上下限阈值、加热状态(HEATING/NO HEAT)
 *   (2) 4 个按键调整最高/最低水温阈值(PB12~PB15, 长按可连发)
 *   (3) 水温低于最低阈值 -> 加热继电器吸合
 *   (4) 水温高于最高阈值 -> 停止加热并在 LCD 提示 "Water too hot"
 *   (5) 缺水开关闭合(PB11 = 高) -> 加水继电器吸合, 缺水指示灯点亮
 *
 * 目标: STM32F103C6T6 (LQFP48, 32KB Flash / 10KB RAM)
 */
#include <stdint.h>

#include "board.h"
#include "stm32f1_regs.h"
#include "bsp.h"
#include "delay.h"
#include "lcd1602.h"
#include "ds18b20.h"
#include "control.h"

static char s_last1[17];
static char s_last2[17];

static uint8_t line_differs(const char *a, const char *b)
{
    uint8_t i;

    for (i = 0u; i < 16u; i++) {
        if (a[i] != b[i]) {
            return 1u;
        }
    }
    return 0u;
}

static void line_save(char *dst, const char *src)
{
    uint8_t i;

    for (i = 0u; i < 16u; i++) {
        dst[i] = src[i];
    }
}

int main(void)
{
    limits_t lim;
    status_t st;
    char     l1[17];
    char     l2[17];
    uint32_t hold_ms[KEY_COUNT];
    uint32_t t_sample;
    uint32_t t_key;
    uint32_t t_now;
    uint8_t  keys_raw;
    uint8_t  keys_stable;
    uint8_t  keys_last;
    uint8_t  short_now;
    uint8_t  i;

    /* ---------------- 初始化 ---------------- */
    bsp_clock_init();               /* HSE 8MHz, 见 board.h            */
    delay_init();                   /* SysTick 1ms                     */
    bsp_outputs_init();             /* 2 路继电器 + 缺水指示灯, 先全部关闭 */
    bsp_inputs_init();              /* 4 个按键 + 缺水开关             */
    lcd_init();
    ds18b20_init();

    control_limits_default(&lim);
    st.valid    = 0u;
    st.heating  = 0u;
    st.filling  = 0u;
    st.overheat = 0u;
    st.temp10   = 0;

    keys_raw    = 0u;
    keys_stable = 0u;
    keys_last   = 0u;
    for (i = 0u; i < (uint8_t)KEY_COUNT; i++) {
        hold_ms[i] = 0u;
    }
    for (i = 0u; i < 17u; i++) {
        s_last1[i] = '\0';
        s_last2[i] = '\0';
    }

    /* ---------------- 开机画面 ---------------- */
    lcd_puts_at(0u, 0u, "  Water Heater  ", 16u);
    lcd_puts_at(1u, 0u, "  STM32F103C6   ", 16u);
    delay_ms(800);

    bsp_outputs_apply(st.heating, st.filling, 0u);
    (void)ds18b20_start_convert();  /* 先启动一次转换                 */

    t_now    = millis();
    t_sample = t_now + SAMPLE_PERIOD_MS;
    t_key    = t_now;

    /* ---------------- 主循环 ---------------- */
    for (;;) {
        t_now = millis();

        /* ---- 缺水开关: 状态一变立刻响应(不用等采样周期) ---- */
        short_now = bsp_sensor_short();
        if (short_now != st.filling) {
            st.filling = short_now;
            bsp_outputs_apply(st.heating, st.filling, short_now);
        }

        /* ---- 按键: 每 20ms 扫描一次, 两拍一致才算稳定 ---- */
        if ((t_now - t_key) >= KEY_SCAN_MS) {
            uint8_t newly;

            t_key    = t_now;
            keys_raw = bsp_keys_read();

            if (keys_raw == keys_last) {
                if (keys_raw != keys_stable) {
                    newly = (uint8_t)(keys_raw & (uint8_t)(~keys_stable));
                    for (i = 0u; i < (uint8_t)KEY_COUNT; i++) {
                        if ((newly & (uint8_t)(1u << i)) != 0u) {
                            control_limits_adjust(&lim, (int)i);
                            hold_ms[i] = 0u;
                        }
                    }
                    keys_stable = keys_raw;
                }
            } else {
                keys_last = keys_raw;
            }

            /* 长按连发 */
            for (i = 0u; i < (uint8_t)KEY_COUNT; i++) {
                if ((keys_stable & (uint8_t)(1u << i)) != 0u) {
                    hold_ms[i] += KEY_SCAN_MS;
                    if (hold_ms[i] >= KEY_REPEAT_DELAY_MS) {
                        hold_ms[i] = (uint32_t)(KEY_REPEAT_DELAY_MS - KEY_REPEAT_MS);
                        control_limits_adjust(&lim, (int)i);
                    }
                } else {
                    hold_ms[i] = 0u;
                }
            }
        }

        /* ---- 每 800ms 读一次温度, 并立刻启动下一次转换 ---- */
        if (time_reached(t_sample)) {
            int16_t raw;
            int16_t temp10;
            uint8_t ok;

            ok     = ds18b20_read_raw(&raw);
            temp10 = (ok != 0u) ? ds18b20_raw_to_tenths(raw) : st.temp10;

            control_update(&st, &lim, temp10, ok, bsp_sensor_short());

#if !ALLOW_HEAT_WHEN_SHORT
            if (short_now != 0u) {
                st.heating = 0u;            /* 缺水时禁止干烧 */
            }
#endif
            bsp_outputs_apply(st.heating, st.filling, short_now);
            (void)ds18b20_start_convert();

            t_sample += SAMPLE_PERIOD_MS;
            if (time_reached(t_sample)) {   /* 万一落后太多, 重新对齐 */
                t_sample = millis() + SAMPLE_PERIOD_MS;
            }
        }

        /* ---- 刷新 LCD(内容变化才写, 避免闪烁) ---- */
        control_line1(l1, &lim, &st);
        control_line2(l2, &st);

        if (line_differs(s_last1, l1) != 0u) {
            lcd_puts_at(0u, 0u, l1, 16u);
            line_save(s_last1, l1);
        }
        if (line_differs(s_last2, l2) != 0u) {
            lcd_puts_at(1u, 0u, l2, 16u);
            line_save(s_last2, l2);
        }
    }
}
