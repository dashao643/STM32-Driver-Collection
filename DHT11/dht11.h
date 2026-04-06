#ifndef __DHT11_H
#define __DHT11_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

#define LED_DEBUG   // LED调试
#define OLED        // OLED显示信息

#define DHT11_READ_INTERVAL_MS      2000      // 读取间隔(单位：毫秒)
#define DHT11_Start_MS              20        // 起始信号拉低时长

typedef enum {
  DHT11_STATE_IDLE = 0,     // 空闲
  DHT11_STATE_START_LOW,    // 拉低 20ms（非阻塞）
  DHT11_STATE_START_HIGH,   // 拉高 13us
  DHT11_STATE_WAIT_ACK,     // 等待DHT响应
  DHT11_STATE_READ_DATA,    // 读取40bit数据
  DHT11_STATE_CHECK,        // 校验数据
} DHT11_StateTypeDef;

typedef struct {
  DHT11_StateTypeDef state;
  uint32_t start_tick;        // 起始信号计时
  uint32_t last_read_tick;    // 上次读取时间
  uint8_t  humidity;          // 湿度
  uint8_t  temperature;       // 温度
  uint8_t  data_buf[5];       // 接收5字节数据
  bool     data_valid;        // 数据是否准备好
} DHT11_HandleTypeDef;

#define DHT11_PIN_LOW()       HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_RESET)
#define DHT11_PIN_HIGH()      HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_SET)
#define DHT11_PIN_READ()      HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin)

void DHT11_Init(void);
void DHT11_Task(void);
bool DHT11_IsDataValid(void);
uint8_t DHT11_GetHumidity(void);
uint8_t DHT11_GetTemperature(void);

#endif
