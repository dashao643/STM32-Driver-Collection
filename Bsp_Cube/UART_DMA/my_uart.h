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
  uint16_t rxIdx;       // 接收缓冲区索引
  uint8_t* txBuf;
  uint16_t txMaxSize;
  bool frameEnd;
} My_UART_t;

typedef enum {
  BLOCK = 0,
  DMA,
} TransmitMode_e;

void UART_Clear(My_UART_t *uart);
void UART_IdleProcess(const USART_TypeDef* Instance, My_UART_t *uart);
void UART_Transmit(const My_UART_t* uart, uint16_t size, TransmitMode_e mode, uint32_t timeout);

void UART_Clear_AT(My_UART_t *uart);
void UART_IdleProcess_AT(const USART_TypeDef* Instance, My_UART_t *uart);

#endif
