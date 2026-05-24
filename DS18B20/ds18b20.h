#ifndef __DS18B20_H
#define __DS18B20_H

#include "ds18b20_def.h"

#include <stdint.h>
#include <stdbool.h>

/********************* ↓选择从机数量↓ *******************/
#define DS18B20_SINGLE_SLAVE              // 单从机模式
// #define DS18B20_MULTIPLE_SLAVES           // 多从机模式(不支持)
/********************* ↑选择从机数量↑ *******************/

// 不支持search rom指令
// 使用VCC供电，不使用寄生电源模式

#define DS18B20_START_US              500        // 起始信号拉低时长：480-780us
#define DS18B20_WAIT_EXIST_US         25         // 起始信号拉高后等待时长：15-60us
#define DS18B20_EXIST_TIMEOUT_US      600        // 检测从机存在超时时间
#define DS18B20_WAIT_RW_BIT_US        63         // 读写bit的总时间：60-65
#define DS18B20_WRITE_BIT_1_US        2          // 写bit 1的拉低时间：1-15
#define DS18B20_WRITE_BIT_0_US        62         // 写bit 0的拉低时间：60-65
#define DS18B20_READ_BIT_START_US     2          // 读信号拉低时间
#define DS18B20_READ_BIT_SPAN_US      15         // 读bit持续时间

#define DS18B20_ROM_SIZE              8          // ROM字节大小，最后一字节CRC
#define DS18B20_SCRATCHPAD_SIZE       9          // RAM字节大小，最后一字节CRC
#define DS18B20_CONFIG_SIZE           3          // 配置寄存器大小

#define DS18B20_CONFIG_TH             50         // 温度上阈值
#define DS18B20_CONFIG_TL             20         // 温度下阈值
#define DS18B20_CONFIG_12Bit          0x7F       // 12bit分辨率，最小读取间隔750ms
#define DS18B20_CONFIG_10Bit          0x3F       // 10bit分辨率，最小读取间隔200ms
#define DS18B20_READ_INTERVAL_MS      2000      // 读取间隔2000ms

typedef struct {
  int8_t tempInt;         // 温度整数部分：-55°C to +125°C
  uint8_t tempDec;        // 温度小数部分：两位小数0 - 75
  uint32_t timer;
} DS18B20_t;

void DS18B20_Init(void);
void DS18B20_Task(void);
void DS18B20_GetTemp(int8_t *tempInt, uint8_t *tempDec);

#endif
