#ifndef __MY_I2C_H__
#define __MY_I2C_H__

#include "stm32f1xx_hal.h" 
#include <stdint.h>

#define I2C_SOFTWARE_DELAY_US      5
#define I2C_SOFTWARE_TIMEOUT_US    50

// 0: OK 
// 1: ERROR
// 2: BUSY
// 3: TIMEOUT
HAL_StatusTypeDef I2C_Mem_Write(uint8_t devAddress, uint16_t memAddress, uint8_t memAddSize, uint8_t *data, uint16_t size);
HAL_StatusTypeDef I2C_Mem_Read(uint8_t devAddress, uint16_t memAddress, uint8_t memAddSize, uint8_t *data, uint16_t size);

#endif
