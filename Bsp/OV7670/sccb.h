#ifndef __SCCB_H__
#define __SCCB_H__

#include "stm32f4xx_hal_def.h"
#include <stdint.h>

// 时钟频率最大400KHz
// 与IIC相同, bit0: 0写, 1读

// SDA 默认配置：开漏输出，高电平，上拉. 
// SCL 默认配置：推挽输出，高电平

#define SCCB_DELAY_US      5
#define SCCB_TIMEOUT_US    100

HAL_StatusTypeDef SCCB_Mem_Write(uint8_t devAddress, uint8_t memAddress, uint8_t data);
HAL_StatusTypeDef SCCB_Mem_Read(uint8_t devAddress, uint8_t memAddress, uint8_t* data);

#endif