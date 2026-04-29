#ifndef __MY_I2C_H__
#define __MY_I2C_H__

#include <stdint.h>
#include <stdbool.h>

#define I2C_SOFTWARE_DELAY_US      5
#define I2C_SOFTWARE_TIMEOUT_US    50

typedef enum {
  I2C_MEMADD_8BIT = 1,
  I2C_MEMADD_16BIT = 2,
} MemAddSize_e;

bool I2C_Mem_Write(uint8_t devAddress, uint16_t memAddress, MemAddSize_e memAddSize, uint8_t *data, uint16_t size);
bool I2C_Mem_Read(uint8_t devAddress, uint16_t memAddress, MemAddSize_e memAddSize, uint8_t *data, uint16_t size);

#endif