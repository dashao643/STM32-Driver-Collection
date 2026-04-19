#include "my_uart.h"

#include <string.h>
#include "gpio.h"

void UART_Channel_Init(UART_ChannelTypeDef* channel, uint8_t* buf, uint16_t size)
{
  channel->buffer = buf;
  channel->bufSize = size;
  UART_Channel_Clear(channel);
}

void UART_Channel_Clear(UART_ChannelTypeDef* channel)
{
  if(channel->buffer != NULL)
    memset(channel->buffer, 0, channel->bufSize);
  channel->index = 0;
  channel->overflow = false;
  channel->frameEnd = false;
}

void UART_Channel_SingleByteProcess(UART_ChannelTypeDef* channel, UART_HandleTypeDef* huart)
{
  if(channel->overflow) 
    return;

  channel->index++;
  if(channel->index < channel->bufSize)
    HAL_UART_Receive_IT(huart, channel->buffer + channel->index, 1);
  else
    channel->overflow = true;
}

void UART_Channel_SetFrameEndFlag(UART_ChannelTypeDef* channel)
{
  channel->frameEnd = true;
}