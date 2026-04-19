#ifndef __MY_UART_H__
#define __MY_UART_H__

#include "usart.h"
#include "stm32f1xx_hal.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct {
  uint8_t* buffer;
  uint16_t bufSize;
  uint16_t index;
  bool overflow;
  bool frameEnd;
} UART_ChannelTypeDef;

void UART_Channel_Init(UART_ChannelTypeDef* channel, uint8_t* buf, uint16_t size);
void UART_Channel_Clear(UART_ChannelTypeDef* channel);
void UART_Channel_SingleByteProcess(UART_ChannelTypeDef* channel, UART_HandleTypeDef* huart);
void UART_Channel_SetFrameEndFlag(UART_ChannelTypeDef* channel);

#endif