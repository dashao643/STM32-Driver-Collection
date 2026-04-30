#include "at24c64.h"
#include "stm32f1xx_hal.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

static uint32_t timer = 0;

/**
 * @brief 判断当前是否处于写入忙时间
 * 
 * @param isWrite 此操作是否为写操作
 * @return true 当前忙，禁止写入
 * @return false 当前空闲
 */
static bool isWriteBusy(bool isWrite)
{
  if((HAL_GetTick() - timer) > AT24C64_WRITE_INTERVAL_MS) {
    // 如果是写操作，刷新时间。读操作不限制
    if(isWrite){
      timer = HAL_GetTick();
    }
    return false;
  }
  return true;
}

/**
 * @brief 指定地址，单字节写入
 * 
 * @param memAddress 指定内存地址：范围 0x0000 - 0x1FFF
 * @param data 字节数据
 * @return HAL_StatusTypeDef 
 */
HAL_StatusTypeDef AT24C64_Write_SingleByte(uint16_t memAddress, uint8_t data)
{
  if (memAddress > AT24C64_MAX_ADDRESS_SPACE) return HAL_ERROR;

  if (isWriteBusy(true)) return HAL_BUSY;

  return I2C_Mem_Write(AT24C64_SLAVE_ADDR, memAddress, 2, &data, 1);
}

/**
 * @brief 页内指定地址，连续多字节写入
 * 
 * @param memAddress 内存地址
 * @param data 数组指针
 * @param size 字节大小 1-32
 * @return HAL_StatusTypeDef 
 */
HAL_StatusTypeDef AT24C64_Write_Byte(uint16_t memAddress, uint8_t *data, uint8_t size)
{
  if (size == 0) return HAL_ERROR;

  if (memAddress + size > AT24C64_MAX_ADDRESS_SPACE) return HAL_ERROR;
  // 如果跨页写入，返回错误
  if(((memAddress % AT24C64_PAGE_SIZE) + size) > AT24C64_PAGE_SIZE) return HAL_ERROR;

  if (isWriteBusy(true)) return HAL_BUSY;

  return I2C_Mem_Write(AT24C64_SLAVE_ADDR, memAddress, 2, data, size);
}

/**
 * @brief 整页字节写入
 * 
 * @param page 页索引 0-255
 * @param data 数组指针
 * @param size 字节大小 1-32
 */
HAL_StatusTypeDef AT24C64_Write_Page(uint16_t page, uint8_t *data, uint8_t size)
{
  if (page > (AT24C64_PAGE_CNT - 1)) return HAL_ERROR;

  if ((size == 0) || (size > AT24C64_PAGE_SIZE)) return HAL_ERROR;

  if (isWriteBusy(true)) return HAL_BUSY;

  uint16_t pageAddr = page * AT24C64_PAGE_SIZE;

  return I2C_Mem_Write(AT24C64_SLAVE_ADDR, pageAddr, 2, data, size);
}

/**
 * @brief 从指定的具体地址读连续字节
 * 
 * @param memAddress 起始内存地址
 * @param data 存放连续字节数据的地址
 * @param size 字节数据大小：0-0x1FFF(从指定地址开始读取的大小不得超过内存地址)
 */
HAL_StatusTypeDef AT24C64_Read_Byte(uint16_t memAddress, uint8_t *data, uint8_t size)
{
  if(size > AT24C64_MAX_READ_SIZE) return HAL_ERROR;

  if((memAddress + size) > AT24C64_MAX_ADDRESS_SPACE) return HAL_ERROR;

  if (isWriteBusy(false)) return HAL_BUSY;

  return I2C_Mem_Read(AT24C64_SLAVE_ADDR, memAddress, 2, data, size);
}

// 整页读取
HAL_StatusTypeDef AT24C64_Read_Page(uint16_t page, uint8_t *data, uint8_t size)
{
  if (page > (AT24C64_PAGE_CNT - 1)) return HAL_ERROR;

  if ((size == 0) || (size > AT24C64_PAGE_SIZE)) return HAL_ERROR;

  if (isWriteBusy(true)) return HAL_BUSY;

  uint16_t pageAddr = page * AT24C64_PAGE_SIZE;

  return I2C_Mem_Read(AT24C64_SLAVE_ADDR, pageAddr, 2, data, size);
}

// 整页擦除
HAL_StatusTypeDef AT24C64_Erase_Page(uint16_t page)
{
  uint8_t buf[AT24C64_PAGE_SIZE] = {0};

  memset(buf, AT24C64_BLANK_BYTE, AT24C64_PAGE_SIZE);

  return AT24C64_Write_Page(page, buf, AT24C64_PAGE_SIZE);
}
