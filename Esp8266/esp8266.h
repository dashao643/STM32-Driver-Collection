#ifndef __ESP8266_H__
#define __ESP8266_H__

#include "my_uart.h"

#include <stdint.h>
#include <stdbool.h>

/********************* ↓选择ESP8266连接模式↓ *******************/
// #define ESP8266_MODE_AP
#define ESP8266_MODE_STA
/********************* ↑选择ESP8266连接模式↑ *******************/

#define ESP8266_INSTANCE                 USART2
#define ESP8266_HANDLE                   &huart2
#define ESP8266_TX_TIMEOUT_MS            50
#define ESP8266_TX_MAXLENTH              50
#define ESP8266_RX_MAXLENTH              256
#define ESP8266_RX_MINLENTH              12       // 最小帧长度
// #define ESP8266_TX_BUFF_MAXLENTH         30       // 回复帧最大帧长

#define ESP8266_RETRY_COUNT              3           // 重试次数   
// #define INITIAL_DELAY_MS                 500      // 上电延时发送
// #define WAIT_RST_DELAY                   800      // 发完RST间隔时间
// #define WAIT_AT_REPLY_MS                 500      // 重试间隔
// #define RETRY_COUNT                      5        // 重试次数

typedef struct {
  My_UART_t uart;
  bool isConfig;
}ESP8266_t;

HAL_StatusTypeDef ESP8266_AT_Transmit(const char *cmd);
HAL_StatusTypeDef ESP8266_AT_Receive(const char *res, uint16_t timeout);

bool ESP8266_ConnectToServer(void);

void ESP8266_Init(void);
void ESP8266_Task(void);
My_UART_t* ESP8266_Get_UART(void);

#endif
