#ifndef __GENERAL_H
#define __GENERAL_H

#include "main.h"

#define CLOCK_FREQUENCY_MHZ     72
#define UARTX_TIMEOUT           5
#define UARTX_PRINTF            &huart1

typedef union
{
  uint8_t bytes[2];
  uint16_t word;
} U16Union;

void Delay_us(__IO uint32_t delay);
uint16_t CRC16_Modbus(uint8_t *buf, uint16_t len);

#endif
