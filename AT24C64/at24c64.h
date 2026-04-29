#ifndef __AT24C64_H
#define __AT24C64_H

#include "main.h"
#include "my_i2c.h"

#include <stdint.h>
#include <stdbool.h>

#define AT24C64_SLAVE_ADDR            0xA0      // 0B1010 0000
#define AT24C64_MAX_ADDRESS_SPACE     0x1FFF    // 0B0001 1111 1111 1111
#define AT24C64_PAGE_SIZE             0x20      // 32 字节
#define AT24C64_PAGE_CNT              0x100     // 256

void AT24C64_Write_Byte(uint16_t memAddress, uint8_t data);
void AT24C64_Write_Page(uint16_t page, uint8_t *data, uint8_t size);
void AT24C64_Read_Byte_Random();
void AT24C64_Read_Byte_Sequential();

#endif
