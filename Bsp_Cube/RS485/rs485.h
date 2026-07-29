#ifndef __RS485_H__
#define __RS485_H__

#include "my_uart.h"

#include <stdint.h>
#include <stdbool.h>

#define RESTORE_TIME_MS         5

#define RS485_TRANSMIT_MODE() HAL_GPIO_WritePin(RS485_CTRL_GPIO_Port, RS485_CTRL_Pin, GPIO_PIN_SET)
#define RS485_RECEIVE_MODE() HAL_GPIO_WritePin(RS485_CTRL_GPIO_Port, RS485_CTRL_Pin, GPIO_PIN_RESET)

typedef struct {
  My_UART_t uart;
  bool transmitFlag;
  uint32_t timer;
}RS485_t;

void RS485_Transmit(RS485_t *rs485, uint16_t size, const TransmitMode_e mode, const uint32_t timeout);
void RS485_Task(RS485_t *rs485);

#endif