#include "stm32f1xx_hal.h"
#include "iap_spi.h"
#include "w25q64.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/*
FLASH地址:        0x8000000 - 0x800FFFF 64KB
Bootloader地址:   0x8000000 - 0x8003FFF 16KB
App代码地址:      0x8004000 - 0x800FFFF 48KB
RAM地址：         0x20000000 - 0x20004FFF
*/

static uint8_t rxBuf[IAP_SPI_RX_BUFF_MAXLENTH];
static uint16_t w25PageCnt = 0;
static uint16_t w25PageIdx = 0;
static void writeFlash(uint16_t flashPageIdx);
static void ramClear(void);
static void errorHandler(uint8_t errorCode);

static void writeFlash(uint16_t flashPageIdx)
{
  uint32_t eraseRes = 0;
  uint32_t addr = IAP_SPI_APP_ADDR + flashPageIdx * FLASH_PAGE_SIZE;

  FLASH_EraseInitTypeDef erase = {0};
  erase.TypeErase = FLASH_TYPEERASE_PAGES;  // 固定
  erase.Banks = FLASH_BANK_1;               // 固定
  erase.NbPages = 1;                        // 一次擦除一页
  erase.PageAddress = addr;                 // 擦除的页地址

  HAL_FLASH_Unlock();
  HAL_FLASHEx_Erase(&erase, &eraseRes);
  
  if(eraseRes != 0xFFFFFFFF){
    HAL_FLASH_Lock();
    errorHandler(IAP_SPI_ERROR_ERASE);
  }
  // 一次写入2Byte, 循环512次, 一次跳2个数
  // for(uint16_t i = 0; i < 1024; i+=2){
  //   uint16_t data = rxBuf[i] | (rxBuf[i + 1] << 8);
  //   HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, data);
  //   addr += 2;
  // }
  // 一次写入4Byte, 循环256次, 一次跳4个数
  for(uint16_t i = 0; i < 1024; i+=4){
    uint32_t data = rxBuf[i] | (rxBuf[i + 1] << 8) | (rxBuf[i + 2] << 16) | (rxBuf[i + 3] << 24);
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, data);
    addr += 4;
  }
  // memset(rxBuf, 0, IAP_SPI_RX_BUFF_MAXLENTH);
  HAL_FLASH_Lock();
}

// 清除iap跳转标志
static void ramClear(void)
{
  *(uint32_t*)IAP_SPI_MAGIC_ADDR = 0;
}

static void errorHandler(uint8_t errorCode)
{
  UNUSED(errorCode);
  // printf("errorCode=%d\n", errorCode);
  // HAL_Delay(1000);
  __disable_irq();
  while (1)
  {
  }
}

/*-----------------------------------------------------------------*/

// true: 表示此次复位是通过app程序收到iap升级指令
// false: 正常复位, 直接跳转
// 此时串口还未初始化, 打印信息移动到run函数
// 内存默认为垃圾值, 不能直接检测是否为0
bool IAP_SPI_RAM_Check(void)
{
  if(*(uint32_t*)IAP_SPI_MAGIC_ADDR != IAP_SPI_MAGIC_VAL)
    return false;

  uint32_t info = *(uint32_t*)IAP_SPI_INFO_ADDR;

  // 高16位
  w25PageCnt = info >> 16;
  // 低16位
  w25PageIdx = info;

  // w25q64 page size = 256, flash page size = 1024: 1024 / 256 = 4
  if(w25PageCnt > 48 * 4)
    errorHandler(IAP_SPI_ERROR_OVERFLOW);

  return true;
}

void IAP_SPI_Run(void)
{
  // printf("w25PageCnt=%d\n", w25PageCnt);
  // printf("w25PageIdx=%d\n", w25PageIdx);

  uint8_t times = 0;
  uint16_t flashPage = 0;

  // 循环读取pageCnt次w25Q64的页空间, 每读取四次, 即总共读取1024字节, 写入一次FLASH
  for(uint16_t i = 0; i < w25PageCnt; i++){
    // 每读取W25Q64四页, 执行一次FLASH写入
    W25Q64_Read_Page(w25PageIdx++, rxBuf + times * 256, 256);
    times++;

    if(times == 4) {
      times = 0;
      writeFlash(flashPage++);
    }
  }
  // printf("times=%d\n", times);
  // 不足4次, 1024总空间的剩余空间补充0xFF写入
  if(times != 0) {
    memset(rxBuf + times * 256, 0xFF, (4 - times) * 256);
    writeFlash(flashPage);
  }

  // 清除标志后软件复位，自动重置所有外设，并跳转app
  ramClear();
  HAL_NVIC_SystemReset();
}

void IAP_SPI_JumpApp(void)
{
  uint32_t appStack = *(__IO uint32_t*)IAP_SPI_APP_ADDR;

  if ((appStack & 0x20000000) == 0)
    errorHandler(IAP_SPI_ERROR_APPSTACK);

  typedef void (*pFunction)(void);
  pFunction Jump_To_Application;

  uint32_t JumpAddress = *(__IO uint32_t*)(IAP_SPI_APP_ADDR + 4);
  Jump_To_Application = (pFunction)JumpAddress;

  __disable_irq();

  __set_MSP(appStack);

  Jump_To_Application();
}
