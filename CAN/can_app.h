#ifndef __CAN_APP_H__
#define __CAN_APP_H__

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#define LED
#define DHT11
#define MY_RTC

// 数据帧寄存器
#define CAN_APP_REMOTE_ACK		     0x0000
#define CAN_APP_LED_RED		         0x0001
#define CAN_APP_LED_GREEN 	       0x0002
#define CAN_APP_LED_BLUE 	         0x0003
#define CAN_APP_PWM_LED	           0x0004

// 支持的操作数
#define CAN_APP_RESET 	           0x00    // 关闭
#define CAN_APP_SET 	             0x01    // 开启
#define CAN_APP_TOGGLE 	           0x02    // 翻转

// 回复帧寄存器
#define CAN_APP_DHT11              0x0005
#define CAN_APP_RTC                0x0006

#define CAN_APP_DHT11_DATA_SIZE    2
#define CAN_APP_RTC_DATA_SIZE      6

bool CAN_APP_DataFrame(uint16_t stdId, const uint8_t* data, uint8_t size);
HAL_StatusTypeDef CAN_APP_RemoteFrame(uint8_t stdId);

#endif
