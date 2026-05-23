#ifndef __DS18B20_H
#define __DS18B20_H

#include "ds18b20_def.h"

#include <stdint.h>
#include <stdbool.h>

// 使用VCC供电，不使用寄生电源模式

#define DS18B20_START_US              500        // 起始信号拉低时长：480-780us
#define DS18B20_WAIT_EXIST_US         25         // 起始信号拉高后等待时长：15-60us
#define DS18B20_EXIST_TIMEOUT_US      600        // 检测从机存在超时时间
#define DS18B20_WAIT_RW_BIT_US        63         // 读写bit的总时间：60-65
#define DS18B20_WRITE_BIT_1_US        2          // 写bit 1的拉低时间：1-15
#define DS18B20_WRITE_BIT_0_US        62         // 写bit 0的拉低时间：60-65
#define DS18B20_READ_BIT_START_US     2          // 读信号拉低时间
#define DS18B20_READ_BIT_SPAN_US      15         // 读bit持续时间

void start(void);
bool exist(void);
void writeByte(uint8_t byte);
uint8_t readByte(void);

#endif
