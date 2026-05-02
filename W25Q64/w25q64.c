#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_def.h"
#include <stdint.h>
#include "w25q64.h"

#define CMD_WRITE_ENABLE                        0x06
#define CMD_WRITE_DISABLE                       0x04
#define CMD_READ_DATA                           0x03
#define CMD_PAGE_PROGRAM                        0x02
#define CMD_SECTOR_ERASE                        0x20
#define CMD_BLOCK_ERASE_32KB                    0x52
#define CMD_BLOCK_ERASE_64KB                    0xD8
#define CMD_READ_STATUS_REG_1                   0x05
#define CMD_WRITE_STATUS_REG_1                  0x01

static inline void W25Q64_CS_LOW(void) 
{
  HAL_GPIO_WritePin(W25Q64_CS_GPIO_PORT, W25Q64_CS_Pin, GPIO_PIN_RESET);
}

static inline void W25Q64_CS_HIGH(void) 
{
  HAL_GPIO_WritePin(W25Q64_CS_GPIO_PORT, W25Q64_CS_Pin, GPIO_PIN_SET);
}

static inline HAL_StatusTypeDef transmit(const uint8_t *data, uint16_t size) 
{
  return HAL_SPI_Transmit(W25Q64_HANDLE, data, size, W25Q64_TIMEOUT_MS);
}

static inline HAL_StatusTypeDef receive(const uint8_t *data, uint16_t size) 
{
  return HAL_SPI_Receive(W25Q64_HANDLE, data, size, W25Q64_TIMEOUT_MS);
}

static void cmdTransmit(const uint8_t cmd)
{
  W25Q64_CS_LOW();
  transmit(&cmd, 1);
  W25Q64_CS_HIGH();
}

static HAL_StatusTypeDef receive(const uint8_t *data, uint16_t size)
{
  W25Q64_CS_LOW();
  uint8_t state = HAL_SPI_Receive(W25Q64_HANDLE, data, size, W25Q64_TIMEOUT_MS);
  W25Q64_CS_HIGH();
  return state;
}

HAL_StatusTypeDef W25Q64_Write_Byte(const uint32_t addr, const uint8_t *data, uint16_t size)
{
  if(addr > W25Q64_MAX_ADDRESS_SPACE) return HAL_ERROR;

  // 发送写使能
  cmdTransmit(CMD_WRITE_ENABLE);

  // 发送三字节地址
  uint8_t addrByte[3] = {addr, addr >> 8, addr >> 16};
  transmit(addrByte, 3);

  // 发送字节数据
  transmit(data, size);

  // 发送写失能
  cmdTransmit(CMD_WRITE_DISABLE);

  // 死等busy，超时返回错误
  return HAL_OK;
}

HAL_StatusTypeDef W25Q64_Write_Page(const uint16_t page, const uint8_t *data, uint16_t size)
{

}

HAL_StatusTypeDef W25Q64_Read_Byte(const uint32_t addr, const uint8_t *data, uint16_t size)
{
  if(addr > W25Q64_MAX_ADDRESS_SPACE) return HAL_ERROR;

  // 发送三字节地址
  uint8_t addrByte[3] = {addr, addr >> 8, addr >> 16};
  transmit(addrByte, 3);

  // 接收字节数据
  return receive(data, size);
}

HAL_StatusTypeDef W25Q64_Erase_Sector(const uint16_t sector)
{

}
