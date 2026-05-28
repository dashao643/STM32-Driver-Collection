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
#define ESP8266_RX_TIMEOUT_MS            20
#define ESP8266_RX_AT_MAXLENTH           20     // 接收AT回复帧最大帧长
#define ESP8266_RX_DATA_MAXLENTH         100    // 接收数据帧最大帧长
// #define ESP8266_RX_BUFF_MINLENTH         12       // 最小帧长度
// #define ESP8266_TX_BUFF_MAXLENTH         30       // 回复帧最大帧长

#define ESP8266_AT_OK_LENGTH             5

// #define INITIAL_DELAY_MS                 500      // 上电延时发送
// #define WAIT_RST_DELAY                   800      // 发完RST间隔时间
// #define WAIT_AT_REPLY_MS                 500      // 重试间隔
// #define RETRY_COUNT                      5        // 重试次数

typedef struct {
  My_UART_t uart;                       // 硬件层成员变量
  // ESP8266_ConfigState_e state;          // 当前状态
  // uint8_t cmdIndex;                     // 需要发送的指令索引
  // ESP8266_AT_ReplyFrame replyFlag;      // 收到回复帧标志
  // uint8_t curRetryCnt;                  // 当前重传次数
  // uint32_t timeTick;                    // 定时器计数
  // bool setConfigFlag;                   // 配置已完成标志
}ESP8266_t;

void ESP8266_AT_Transmit(const char *cmd);
void ESP8266_AT_Receive(uint8_t len);

void ESP8266_Init(void);
void ESP8266_Task(void);
My_UART_t* ESP8266_Get_UART(void);

#endif
