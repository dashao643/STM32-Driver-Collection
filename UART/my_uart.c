#include "my_uart.h"
#include "gpio.h"

#include <stdint.h>
#include <string.h>

void UART_Clear(My_UART_t *uart)
{
  if(uart->rxBuf)
    memset(uart->rxBuf, 0, uart->rxMaxSize);
  uart->rxSize = 0;

  uart->frameEnd = false;
}

void UART_IdleProcess(USART_TypeDef* Instance, My_UART_t *uart)
{
  if(Instance != uart->instance)  
    return;

  LED_RED_TOGGLE();
  if(__HAL_UART_GET_FLAG(uart->handle, UART_FLAG_IDLE) == SET){
      __HAL_UART_CLEAR_IDLEFLAG(uart->handle);

    // 计算收到的字节数
    uart->rxSize = uart->rxMaxSize - __HAL_DMA_GET_COUNTER(uart->handle->hdmarx);

    if(uart->rxSize > 0)
      uart->frameEnd = true;

    HAL_UART_AbortReceive(uart->handle);
    HAL_UART_Receive_DMA(uart->handle, uart->rxBuf, uart->rxMaxSize);
  }
}
