#include "esp8266.h"

#include "gpio.h"
#include "stdio.h"
#include <stdint.h>
#include "modbus.h"

static UART_ChannelTypeDef esp8266;
static uint8_t rx_buf[ESP8266_RX_BUFF_MAXLENTH];

static uint8_t esp8266_tx_buff[ESP8266_TX_BUFF_MAXLENTH];
static uint8_t esp8266_tx_index = 0;

static void frameReply(void)
{
  HAL_UART_Transmit(ESP8266_UARTX, esp8266_tx_buff, esp8266_tx_index, ESP8266_UARTX_TIMEOUT);
  esp8266_tx_index = 0;
}
    // // 只有缓冲区里包含OK/ERROR，才是ESP真实回复，才打印
    // if(strstr((char*)esp8266.buffer, "OK") != NULL || 
    //    strstr((char*)esp8266.buffer, "ERROR") != NULL)
    // {
    //     printf("%s\n",esp8266.buffer);
    // }
    // // 自己串扰回来的AT指令，直接丢弃不打印
static void frameProcess(void)
{
  // 数据回显,通过串口1发送
  printf("%s\n",esp8266.buffer);
  // HAL_UART_Transmit(MODBUS_UARTX, esp8266.buffer, esp8266.index, ESP8266_UARTX_TIMEOUT);
}

void ESP8266_Init(void)
{
  UART_Channel_Init(&esp8266,rx_buf,ESP8266_RX_BUFF_MAXLENTH);
  __HAL_UART_ENABLE_IT(ESP8266_UARTX,UART_IT_IDLE);
  HAL_UART_Receive_IT(ESP8266_UARTX, esp8266.buffer, 1);
}

void ESP8266_SingleByteProcess(void)
{
  UART_Channel_SingleByteProcess(&esp8266, ESP8266_UARTX);
}

void ESP8266_SetFrameEndFlag(void)
{
  UART_Channel_SetFrameEndFlag(&esp8266);
}

void ESP8266_Task(void)
{
  if(esp8266.frameEnd){
    frameProcess();
    LED_RED_TOGGLE();
    HAL_UART_AbortReceive_IT(ESP8266_UARTX);
    UART_Channel_Clear(&esp8266);
    HAL_UART_Receive_IT(ESP8266_UARTX, esp8266.buffer, 1);
  }
}

void ESP8266_AT_Transmit(char cmd[],uint16_t len)
{
  HAL_UART_Transmit(ESP8266_UARTX, (uint8_t*)cmd, len, ESP8266_UARTX_TIMEOUT);
}