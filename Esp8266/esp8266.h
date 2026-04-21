#ifndef __ESP8266_H__
#define __ESP8266_H__

#include "my_uart.h"
#include "stm32f1xx.h"
#include "stm32f1xx_hal.h"

#include <stdint.h>
#include <stdbool.h>

#define ESP8266_INSTANCE                 USART2
#define ESP8266_UARTX                    &huart2
#define ESP8266_UARTX_TIMEOUT            500
#define ESP8266_RX_BUFF_MAXLENTH         100      // 最大帧长度
#define ESP8266_RX_BUFF_MINLENTH         12       // 最小帧长度

// #define ESP8266_TX_BUFF_MAXLENTH         64       // 回复帧最大帧长

#define INITIAL_DELAY_MS                 500      // 上电延时发送
#define WAIT_RST_DELAY                   800      // 发完RST间隔时间
#define WAIT_AT_REPLY_MS                 500      // 重试间隔
#define RETRY_COUNT                      5        // 重试次数

typedef enum{
  NONE = 0,
  AT_OK,
  AT_ERROR,
}ESP8266_AT_ReplyFrame;

typedef enum{
  AT_INITIAL = 0,
  AT_SEND,
  AT_WAIT_OK,
  AT_SUCCESS,
  AT_FAIL,
}ESP8266_ConfigState;

typedef struct {
  ESP8266_ConfigState state;            // 当前状态
  uint8_t cmdIndex;                     // 需要发送的指令索引
  ESP8266_AT_ReplyFrame replyFlag;      // 收到回复帧标志
  uint8_t curRetryCnt;                  // 当前重传次数
  uint32_t timeTick;                    // 定时器计数
  bool setConfigFlag;                   // 配置已完成标志
}ESP8266_HandleTypeDef;

// 定义一个函数指针类型：无参数、无返回值
typedef void (*CmdFunc_t)(void);

void ESP8266_Init(void);
void ESP8266_SingleByteProcess(void);
void ESP8266_SetFrameEndFlag(void);
void ESP8266_Task(void);

void ESP8266_AT_Transmit(const char cmd[]);

#endif