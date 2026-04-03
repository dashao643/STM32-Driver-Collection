#ifndef __MODBUS_USART_H__
#define __MODBUS_USART_H__

#include <stdint.h>
#include <stdbool.h>

#define RX_BUFF_MAXLENTH 64
#define USART1_TIMEOUT 500

typedef struct{
  uint8_t tempByte;
  uint8_t buffer[RX_BUFF_MAXLENTH];
  uint16_t index;
  // bool byteCompleted;
}modbus_usart_rx_t;

void Modbus_UART_Init(void);
void Modbus_UART_SingleByte(void);
void Modbus_UART_FrameEnd(void);

#endif