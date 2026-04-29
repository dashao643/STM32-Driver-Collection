#include "at24c64.h"
#include <stdint.h>


void AT24C64_Write_Byte(uint16_t memAddress, uint8_t data)
{
  if(memAddress > AT24C64_MAX_ADDRESS_SPACE) return;

  I2C_Mem_Write(AT24C64_SLAVE_ADDR, memAddress, I2C_MEMADD_16BIT, &data, 1);
}

/**
 * @brief 整页字节写入
 * 
 * @param page 页索引 0-255
 * @param data 数组指针
 * @param size 字节大小 1-32
 */
void AT24C64_Write_Page(uint16_t page, uint8_t *data, uint8_t size)
{
  if (page > AT24C64_PAGE_CNT - 1) return;
  if ((size == 0) || (size > AT24C64_PAGE_SIZE)) return;
  
  uint16_t pageAddr = page * AT24C64_PAGE_SIZE;
  I2C_Mem_Write(AT24C64_SLAVE_ADDR, pageAddr, I2C_MEMADD_16BIT, data, size);
}

void AT24C64_Read_Byte_Random()
{

}

void AT24C64_Read_Byte_Sequential()
{

}