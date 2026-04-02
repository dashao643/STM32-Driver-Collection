#ifndef __DHT11_H
#define __DHT11_H

#include "main.h"

#define LED_DEBUG             // LED调试
#define OLED_DEBUG            // OLED显示调试信息
// #define USART1_DEBUG          // 串口打印调试信息

#define CLOCK_FREQUENCY_MHZ 72

#define DHT11_LOW() HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_RESET)
#define DHT11_HIGH() HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_SET)

#define DHT11_PIN_READ() HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin)

void DHT11_ReadData(uint8_t *temp, uint8_t *humi);

void DHT11_Delay_us(uint32_t us);        

#endif
