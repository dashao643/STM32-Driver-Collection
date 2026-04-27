#ifndef __MY_I2C_H__
#define __MY_I2C_H__

#include <stdint.h>
#include <stdbool.h>

#define I2C_SOFTWARE_DELAY_US      5
#define I2C_SOFTWARE_TIMEOUT_MS    50

bool I2C_Transmit(uint8_t devAddress, uint8_t *data, uint16_t size);
bool I2C_Receive(uint8_t devAddress, uint8_t *data, uint16_t size);
bool I2C_Mem_Write(uint8_t devAddress, uint8_t memAddress, uint8_t memAddSize, uint8_t *data, uint16_t size);
bool I2C_Mem_Read(uint8_t devAddress, uint8_t memAddress, uint8_t memAddSize, uint8_t *data, uint16_t size);

#endif