#ifndef __ESP8266_H__
#define __ESP8266_H__

#include "my_uart.h"
#include "stm32f1xx.h"
#include "stm32f1xx_hal.h"
#include <stdint.h>

#define ESP8266_INSTANCE                 USART2
#define ESP8266_UARTX                    &huart2
#define ESP8266_UARTX_TIMEOUT            500
#define ESP8266_RX_BUFF_MAXLENTH         100      // 最大帧长度
#define ESP8266_RX_BUFF_MINLENTH         10       // 最小帧长度

#define ESP8266_TX_BUFF_MAXLENTH         64      // 回复帧最大帧长

void ESP8266_Init(void);
void ESP8266_SingleByteProcess(void);
void ESP8266_SetFrameEndFlag(void);
void ESP8266_Task(void);

void ESP8266_AT_Transmit(char cmd[],uint16_t len);

#endif