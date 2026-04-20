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
#define ESP8266_RX_BUFF_MINLENTH         10       // 最小帧长度

// #define ESP8266_TX_BUFF_MAXLENTH         64       // 回复帧最大帧长

#define INITIAL_DELAY_MS                 500      // 上电延时发送
#define WAIT_RST_DELAY                   800      // 发完RST间隔时间
#define WAIT_AT_REPLY_MS                 500      // 重试间隔
#define RETRY_COUNT                      5        // 重试次数

// 指令表字符串数组
const char AT_CmdList[4][]{
  {"AT\r\n"},
  {"AT+RST\r\n"},
  {"AT+CIPMUX=1\r\n"},
  {"AT+CIPSERVER=1,80\r\n"}
};

typedef enum{
  NONE = 0,
  AT_OK,
  AT_ERROR,
  AT_READY,
}ESP8266_AT_ReplyFrame;

typedef enum{
  INITIAL = 0,
  SEND_AT,
  WAIT_AT_OK,
  SEND_RST,
  WAIT_RST,
  SET_MODE,
  WAIT_MODE,
  SET_WiFi_CONFIG,
  WAIT_WiFi,
  SET_CIPMUX,
  WAIT_CIPMUX,
  START_SERVICE,
  WAIT_SERVICE,
  FINISH,
  FAIL,
}ESP8266_ConfigState;

typedef struct {
  ESP8266_ConfigState state;            // 当前状态
  ESP8266_AT_ReplyFrame replyFlag;      // 收到回复帧标志
  uint8_t curRetryCnt;                  // 当前重传次数
  uint32_t timeTick;                    // 定时器计数
  bool setInfoFlag;                     // 配置完成后发送一次信息
}ESP8266_HandleTypeDef;

void ESP8266_Init(void);
void ESP8266_SingleByteProcess(void);
void ESP8266_SetFrameEndFlag(void);
void ESP8266_Task(void);

void ESP8266_AT_Transmit(char cmd[]);

#endif