#ifndef __MY_UART_H__
#define __MY_UART_H__

#include "usart.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct {
  UART_HandleTypeDef *handle;
  USART_TypeDef* instance;
  uint8_t* rxBuf;
  uint16_t rxMaxSize;
  uint16_t rxSize;
  uint8_t* txBuf;
  uint16_t txMaxSize;
  bool frameEnd;
} My_UART_t;

void UART_Clear(My_UART_t *uart);
void UART_IdleProcess(USART_TypeDef* Instance, My_UART_t *uart);

#endif