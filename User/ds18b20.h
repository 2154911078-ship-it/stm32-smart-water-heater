/*
 * ds18b20.h  --  DS18B20 单总线温度传感器驱动
 */
#ifndef DS18B20_H
#define DS18B20_H

#include <stdint.h>

void    ds18b20_init(void);
/* 启动一次温度转换, 返回 1 = 器件有应答 */
uint8_t ds18b20_start_convert(void);
/* 读暂存器前两字节(温度), 返回 1 = 读取成功; raw 为 1/16℃ 有符号数 */
uint8_t ds18b20_read_raw(int16_t *raw);
/* 1/16℃ -> 0.1℃ */
int16_t ds18b20_raw_to_tenths(int16_t raw);

#endif /* DS18B20_H */
