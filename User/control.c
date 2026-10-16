/*
 * control.c  --  控制策略与 LCD 显示内容
 * 只使用 stdint.h, 不调用任何库函数, 可在 PC 上单独编译测试。
 */
#include "control.h"

#define LCD_COLS    16u

/* ---------------- 小工具(不用 libc) ---------------- */
static void put_str(char *dst, const char *src)
{
    while (*src != '\0') {
        *dst++ = *src++;
    }
    *dst = '\0';
}

static void pad_to(char *buf, uint8_t len)
{
    uint8_t n = 0u;

    while (buf[n] != '\0') {
        n++;
    }
    while (n < len) {
        buf[n] = ' ';
        n++;
    }
    buf[len] = '\0';
}

/* 把 0.1℃ 的温度格式化成固定 5 个字符, 右对齐: " 25.5" / "-5.5" / "125.0" */
static void temp_field(char *w, int16_t temp10)
{
    char    tmp[8];
    uint8_t n = 0u;
    uint8_t i;
    uint8_t neg = 0u;
    int32_t v = (int32_t)temp10;

    if (v < 0) {
        neg = 1u;
        v = -v;
    }

    tmp[n++] = (char)('0' + (char)(v % 10));    /* 小数位 */
    tmp[n++] = '.';
    v /= 10;
    do {
        tmp[n++] = (char)('0' + (char)(v % 10));
        v /= 10;
    } while (v != 0);

    if (neg != 0u) {
        tmp[n++] = '-';
    }

    for (i = 0u; i < 5u; i++) {
        w[i] = ' ';
    }
    for (i = 0u; (i < n) && (i < 5u); i++) {
        w[4u - i] = tmp[i];                     /* tmp 里是倒序, 反过来填 */
    }
    w[5] = '\0';
}

/* ---------------- 阈值 ---------------- */
void control_limits_default(limits_t *l)
{
    l->low  = TEMP_LOW_DEFAULT;
    l->high = TEMP_HIGH_DEFAULT;
}

void control_limits_adjust(limits_t *l, int key)
{
    switch (key) {
    case KEY_L_PLUS:
        if (l->low < (int16_t)TEMP_LOW_MAX) {
            l->low++;
            if (l->low >= l->high) {            /* 下限不能超过上限 */
                l->low = (int16_t)(l->high - 1);
            }
        }
        break;

    case KEY_L_MINUS:
        if (l->low > (int16_t)TEMP_LOW_MIN) {
            l->low--;
        }
        break;

    case KEY_H_PLUS:
        if (l->high < (int16_t)TEMP_HIGH_MAX) {
            l->high++;
        }
        break;

    case KEY_H_MINUS:
        if (l->high > (int16_t)TEMP_HIGH_MIN) {
            l->high--;
            if (l->high <= l->low) {            /* 上限不能低于下限 */
                l->high = (int16_t)(l->low + 1);
            }
        }
        break;

    default:
        break;
    }
}

/* ---------------- 控制 ---------------- */
void control_update(status_t *st, const limits_t *l,
                    int16_t temp10, uint8_t valid, uint8_t water_short)
{
    st->valid  = (uint8_t)(valid ? 1u : 0u);
    st->temp10 = temp10;
    st->filling = (uint8_t)(water_short ? 1u : 0u);

    if (valid != 0u) {
        if (temp10 < (int16_t)(l->low * 10)) {
            st->heating = 1u;                   /* (3) 低于下限 -> 开始加热 */
        } else if (temp10 > (int16_t)(l->high * 10)) {
            st->heating = 0u;                   /* (4) 高于上限 -> 停止加热 */
        }
        /* 上、下限之间保持原状态(滞回), 避免继电器频繁动作 */
    }

    st->overheat = (uint8_t)((valid != 0u) && (temp10 > (int16_t)(l->high * 10)));
}

/* ---------------- 显示 ---------------- */
/* 第 1 行:  T: 25.5C L40 H60   (当前水温 + 下限 + 上限) */
void control_line1(char *buf, const limits_t *l, const status_t *st)
{
    char t[6];
    uint8_t i;

    if (st->valid != 0u) {
        temp_field(t, st->temp10);
    } else {
        t[0] = ' '; t[1] = ' '; t[2] = '-'; t[3] = '-'; t[4] = '-'; t[5] = '\0';
    }

    buf[0] = 'T';
    buf[1] = ':';
    for (i = 0u; i < 5u; i++) {
        buf[2u + i] = t[i];
    }
    buf[7]  = 'C';
    buf[8]  = ' ';
    buf[9]  = 'L';
    buf[10] = (char)('0' + (char)(l->low / 10));
    buf[11] = (char)('0' + (char)(l->low % 10));
    buf[12] = ' ';
    buf[13] = 'H';
    buf[14] = (char)('0' + (char)(l->high / 10));
    buf[15] = (char)('0' + (char)(l->high % 10));
    buf[16] = '\0';
}

/* 第 2 行:
 *   超温   : "Water too hot"        (题目要求 4)
 *   其它   : 左 8 字符 = 加热状态(HEATING / NO HEAT)
 *            右 8 字符 = 加水状态(FILLING / NO FILL)          */
void control_line2(char *buf, const status_t *st)
{
    if (st->overheat != 0u) {
        put_str(buf, "Water too hot");
        pad_to(buf, (uint8_t)LCD_COLS);
        return;
    }

    put_str(buf, (st->heating != 0u) ? "HEATING " : "NO HEAT ");
    put_str(&buf[8], (st->filling != 0u) ? "FILLING " : "NO FILL ");
    buf[LCD_COLS] = '\0';
}
