#ifndef __W25Q64_H
#define __W25Q64_H

#include "spi.h"
#include <stdint.h>

// 总大小: 64MBit -> 8MB

#define W25Q64_INSTANCE                 SPI1
#define W25Q64_HANDLE                   &hspi1
#define W25Q64_CS_GPIO_PORT             SPI1_CS1_GPIO_Port
#define W25Q64_CS_Pin                   SPI1_CS1_Pin
#define W25Q64_TIMEOUT_MS               100
#define W25Q64_BUSY_BLOCK_MS            200            // 阻塞等待busy

#define W25Q64_MAX_ADDRESS_SPACE        0x7FFFFF
#define W25Q64_PAGE_SIZE                256            // 页大小：byte
#define W25Q64_PAGE_CNT                 32768          // 页数
#define W25Q64_SECTOR_SIZE              4096           // 扇区大小
#define W25Q64_SECTOR_CNT               2048           // 扇区数

HAL_StatusTypeDef W25Q64_Write_Byte(uint16_t page, uint16_t addrInPage, const uint8_t *data, uint16_t size);
HAL_StatusTypeDef W25Q64_Write_Page(uint16_t page, const uint8_t *data, uint16_t size);
HAL_StatusTypeDef W25Q64_Write_Sector(uint16_t sector, const uint8_t *data, uint16_t size);

HAL_StatusTypeDef W25Q64_Read_Byte(uint16_t page, uint16_t addrInPage, uint8_t *data, uint16_t size);
HAL_StatusTypeDef W25Q64_Read_Page(uint16_t page, uint8_t *data, uint16_t size);
HAL_StatusTypeDef W25Q64_Read_Sector(uint16_t sector, uint8_t *data, uint16_t size);

HAL_StatusTypeDef W25Q64_Erase_Sector(uint16_t sector);

#endif
