/*
 * control.h  --  控制策略与 LCD 显示内容(纯逻辑, 不碰硬件, 方便单独测试)
 *
 * 规则(与题目要求一致):
 *   (2) 最高/最低水温阈值可用按键调整
 *   (3) 水温 < 最低水温  -> 加热继电器吸合
 *       水温 > 最高水温  -> 加热继电器断开(滞回, 中间区间保持原状态)
 *   (4) 水温 > 最高水温  -> LCD 提示 "Water too hot"
 *   (5) 缺水开关闭合     -> 加水继电器吸合
 */
#ifndef CONTROL_H
#define CONTROL_H

#include <stdint.h>

/* 阈值默认值与范围(整数摄氏度) */
#define TEMP_LOW_DEFAULT    40
#define TEMP_HIGH_DEFAULT   60
#define TEMP_LOW_MIN        10
#define TEMP_LOW_MAX        89
#define TEMP_HIGH_MIN       11
#define TEMP_HIGH_MAX       90

/* 4 个按键的序号, 与 bsp_keys_read() 的 bit0~bit3 一一对应 */
enum {
    KEY_L_PLUS = 0,     /* 温度下限 + */
    KEY_L_MINUS,        /* 温度下限 - */
    KEY_H_PLUS,         /* 温度上限 + */
    KEY_H_MINUS,        /* 温度上限 - */
    KEY_COUNT
};

typedef struct {
    int16_t low;        /* 最低水温阈值 (℃) */
    int16_t high;       /* 最高水温阈值 (℃) */
} limits_t;

typedef struct {
    uint8_t valid;      /* 温度是否有效(传感器是否应答)     */
    uint8_t heating;    /* 加热继电器状态                   */
    uint8_t filling;    /* 加水继电器状态                   */
    uint8_t overheat;   /* 水温高于上限(提示 Water too hot) */
    int16_t temp10;     /* 当前水温, 单位 0.1℃              */
} status_t;

void control_limits_default(limits_t *l);
void control_limits_adjust(limits_t *l, int key);

/* 每个采样周期调用一次 */
void control_update(status_t *st, const limits_t *l,
                    int16_t temp10, uint8_t valid, uint8_t water_short);

/* 生成 LCD 两行内容, 各 16 个字符 + '\0' */
void control_line1(char *buf, const limits_t *l, const status_t *st);
void control_line2(char *buf, const status_t *st);

#endif /* CONTROL_H */
