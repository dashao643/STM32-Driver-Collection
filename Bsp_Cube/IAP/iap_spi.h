#ifndef __IAP_SPI_H__
#define __IAP_SPI_H__

#include <stdbool.h>

// 通过SPI读取外部FLASH将代码写入内部FLASH
// FLASH页大小1024B, 共64页

#define IAP_SPI_BOOTLOADER_SIZE           0x4000               // 16KB

#define IAP_SPI_RX_BUFF_MAXLENTH          FLASH_PAGE_SIZE      // 数据包长度为FLASH页大小(1024B)

#define IAP_SPI_MAGIC_ADDR                0x20004FFC           // 标志存储地址，RAM最后四个字节
#define IAP_SPI_MAGIC_VAL                 0xA5A5A5A5           // 从app程序跳转标志

#define IAP_SPI_INFO_ADDR                 0x20004FF8           // 存储程序占用页数(uint16_t)和起始页索引(uint16_t)

#define IAP_SPI_APP_ADDR                  FLASH_BASE + IAP_SPI_BOOTLOADER_SIZE  // 0x08004000

#define IAP_SPI_ERROR_OVERFLOW            0xFF                 // 程序包溢出
#define IAP_SPI_ERROR_ERASE               0xFE                 // FLASH擦除错误
#define IAP_SPI_ERROR_APPSTACK            0xFD                 // APP程序栈指针错误

bool IAP_SPI_RAM_Check(void);
void IAP_SPI_Run(void);
void IAP_SPI_JumpApp(void);

#endif
