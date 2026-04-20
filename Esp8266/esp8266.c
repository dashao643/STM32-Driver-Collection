#include "esp8266.h"

#include "modbus.h"
#include "gpio.h"
#include "stm32f1xx_hal.h"

#include <stdio.h>
#include <string.h>


static uint8_t rx_buf[ESP8266_RX_BUFF_MAXLENTH];
static UART_ChannelTypeDef esp8266_uart;
static ESP8266_HandleTypeDef esp8266_handle;

// static uint8_t esp8266_tx_buff[ESP8266_TX_BUFF_MAXLENTH];
// static uint8_t esp8266_tx_index = 0;

static void stateClear(void)
{
  esp8266_handle.curRetryCnt = 0;
  esp8266_handle.replyFlag = NONE;
  esp8266_handle.timeTick = HAL_GetTick();
}

static void AT_Transmit(char cmd[])
{
  uint16_t len = strlen(cmd);
  HAL_UART_Transmit(ESP8266_UARTX, (uint8_t*)cmd, len, ESP8266_UARTX_TIMEOUT);
}

static void AT_SetReplyFlag(void)
{
  if(strstr((char*)esp8266_uart.buffer, "OK")){
    esp8266_handle.replyFlag = AT_OK;
    return;
  }

  if(strstr((char*)esp8266_uart.buffer, "ERROR")){
    esp8266_handle.replyFlag = AT_ERROR;
    return;
  }

  if(strstr((char*)esp8266_uart.buffer, "ready")){
    esp8266_handle.replyFlag = AT_READY;
    return;
  }
}

static void AT_Config(void)
{
  // printf("%d ",configState);
  switch (esp8266_handle.state){
  case INITIAL:{
    if((HAL_GetTick() - esp8266_handle.timeTick) > INITIAL_DELAY_MS){
      esp8266_handle.timeTick = HAL_GetTick();
      esp8266_handle.state = SEND_AT;
    }
    break;
  }
  case SEND_AT:{
    if(esp8266_handle.curRetryCnt == RETRY_COUNT){
      esp8266_handle.state = FAIL;
      return;
    }
    esp8266_handle.curRetryCnt++;
    AT_Transmit("AT\r\n");
    esp8266_handle.state = WAIT_AT_OK;
    esp8266_handle.timeTick = HAL_GetTick();
    break;
  }
  case WAIT_AT_OK:{
    if(esp8266_handle.replyFlag == AT_OK){
      esp8266_handle.state = SEND_RST;
      stateClear();
    }
    if((HAL_GetTick() - esp8266_handle.timeTick) > WAIT_AT_REPLY_MS){
      esp8266_handle.state = SEND_AT;
      esp8266_handle.timeTick = HAL_GetTick();
    }
    break;
  }
  // 重启只发一次
  case SEND_RST:{
    AT_Transmit("AT+RST\r\n");
    esp8266_handle.timeTick = HAL_GetTick();
    esp8266_handle.state = WAIT_RST;
    break;
  }
  case WAIT_RST:{
    if((HAL_GetTick() - esp8266_handle.timeTick) > WAIT_RST_DELAY){
      esp8266_handle.state = SET_CIPMUX;
      esp8266_handle.timeTick = HAL_GetTick();
    }
    break;
  }
  case SET_CIPMUX:{
    if(esp8266_handle.curRetryCnt == RETRY_COUNT){
      esp8266_handle.state = FAIL;
      return;
    }
    esp8266_handle.curRetryCnt++;
    AT_Transmit("AT+CIPMUX=1\r\n");
    esp8266_handle.state = WAIT_CIPMUX;
    esp8266_handle.timeTick = HAL_GetTick();
    break;
  }
  case WAIT_CIPMUX:{
    if(esp8266_handle.replyFlag == AT_OK){
      esp8266_handle.state = START_SERVICE;
      stateClear();
    }
    if((HAL_GetTick() - esp8266_handle.timeTick) > WAIT_AT_REPLY_MS){
      esp8266_handle.state = SET_CIPMUX;
      esp8266_handle.timeTick = HAL_GetTick();
    }
    break;
  }
  case START_SERVICE:{
    if(esp8266_handle.curRetryCnt == RETRY_COUNT){
      esp8266_handle.state = FAIL;
      return;
    }
    esp8266_handle.curRetryCnt++;
    AT_Transmit("AT+CIPSERVER=1,80\r\n");
    // AT_Transmit("AT+DASHAO=1,80\r\n");
    esp8266_handle.state = WAIT_SERVICE;
    esp8266_handle.timeTick = HAL_GetTick();
    break;
  }
  case WAIT_SERVICE:{
    if(esp8266_handle.replyFlag == AT_OK){
      esp8266_handle.state = FINISH;
      stateClear();
    }
    if((HAL_GetTick() - esp8266_handle.timeTick) > WAIT_AT_REPLY_MS){
      esp8266_handle.state = START_SERVICE;
      esp8266_handle.timeTick = HAL_GetTick();
    }
    break;
  }
  case FINISH:{
    LED_GREEN_TOGGLE();
    esp8266_handle.setInfoFlag = true;
    break;
  }
  case FAIL:{
    LED_RED_TOGGLE();
    esp8266_handle.setInfoFlag = true;
    break;
  }
  default:
    break;
  }
}

static void frameProcess(void)
{
  // 设置重启后不解析数据
  if(esp8266_handle.state == WAIT_RST) return;

  printf("%s\n",esp8266_uart.buffer);

  if(!esp8266_handle.setInfoFlag) {
    AT_SetReplyFlag();
  }
}

// .c文件里定义的数组，传入结构体，使用结构体里的数组
void ESP8266_Init(void)
{
  UART_Channel_Init(&esp8266_uart,rx_buf,ESP8266_RX_BUFF_MAXLENTH);
  __HAL_UART_ENABLE_IT(ESP8266_UARTX, UART_IT_IDLE);
  HAL_UART_Receive_IT(ESP8266_UARTX, esp8266_uart.buffer, 1);
  esp8266_handle.state = INITIAL;
  esp8266_handle.replyFlag = NONE;
  esp8266_handle.curRetryCnt = 0;
  esp8266_handle.timeTick = HAL_GetTick();
  esp8266_handle.setInfoFlag = false;
}

void ESP8266_SingleByteProcess(void)
{
  UART_Channel_SingleByteProcess(&esp8266_uart, ESP8266_UARTX);
}

void ESP8266_SetFrameEndFlag(void)
{
  UART_Channel_SetFrameEndFlag(&esp8266_uart);
}

void ESP8266_Task(void)
{
  if(!esp8266_handle.setInfoFlag){
    AT_Config();
  }
  if(esp8266_uart.frameEnd){
    frameProcess();
    HAL_UART_AbortReceive_IT(ESP8266_UARTX);
    UART_Channel_Clear(&esp8266_uart);
    HAL_UART_Receive_IT(ESP8266_UARTX, esp8266_uart.buffer, 1);
  }
}

void ESP8266_AT_Transmit(char cmd[])
{
  uint16_t len = strlen(cmd);
  HAL_UART_Transmit(ESP8266_UARTX, (uint8_t*)cmd, len, ESP8266_UARTX_TIMEOUT);
}