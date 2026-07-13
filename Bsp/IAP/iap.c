#include "stm32f1xx_hal.h"
#include "iap.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#if defined IAP_MODEL_FROM_DIR_UART
#include "usart.h"
#elif defined IAP_MODEL_FROM_EXT_FLASH
#include "w25q64.h"
#endif

/*
FLASH地址:        0x8000000 - 0x800FFFF 64KB
Bootloader地址:   0x8000000 - 0x8003FFF 16KB
App代码地址:      0x8004000 - 0x800FFFF 48KB
RAM地址：         0x20000000 - 0x20004FFF
*/

static IAP_e state = IAP_SEND_ACK;
static uint8_t rxBuf[IAP_RX_BUFF_MAXLENTH];
static uint8_t packageCnt = 0;

static void response(uint8_t byte);
static bool waitPackage(void);
static void writeFlash(void);
static void ramClear(void);
static void errorHandler(uint8_t errorCode);

static void response(uint8_t byte)
{
#ifdef IAP_MODEL_FROM_DIR_UART
  HAL_UART_Transmit(IAP_HANDLE, &byte, 1, IAP_TX_TIMEOUT);
#endif 
}

// true: 等待成功, false: 等待超时
static bool waitPackage(void)
{
#ifdef IAP_MODEL_FROM_DIR_UART
  if(HAL_UART_Receive(IAP_HANDLE, rxBuf, IAP_RX_BUFF_MAXLENTH, IAP_RX_TIMEOUT) == HAL_OK){
    return true;
  }
#endif 
  return false;
}

static void writeFlash(void)
{
  // 从零开始地址, 等于48时再执行即溢出
  if(packageCnt >= 48){
    response(IAP_ERROR_OVERFLOW);
    errorHandler(IAP_ERROR_OVERFLOW);
  }

  uint32_t eraseRes = 0;
  uint32_t addr = IAP_APP_ADDR + packageCnt * FLASH_PAGE_SIZE;

  FLASH_EraseInitTypeDef erase = {0};
  erase.TypeErase = FLASH_TYPEERASE_PAGES;  // 固定
  erase.Banks = FLASH_BANK_1;               // 固定
  erase.NbPages = 1;                        // 一次擦除一页
  erase.PageAddress = addr;                 // 擦除的页地址

  HAL_FLASH_Unlock();
  HAL_FLASHEx_Erase(&erase, &eraseRes);
  
  if(eraseRes != 0xFFFFFFFF){
    HAL_FLASH_Lock();
    response(IAP_ERROR_ERASE);
    errorHandler(IAP_ERROR_ERASE);
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
  memset(rxBuf, 0, IAP_RX_BUFF_MAXLENTH);
  HAL_FLASH_Lock();

  packageCnt++;
}

// 清除iap跳转标志
static void ramClear(void)
{
  *(uint32_t*)IAP_MAGIC_ADDR = 0;
}

static void errorHandler(uint8_t errorCode)
{
  UNUSED(errorCode);
  __disable_irq();
  while (1)
  {
  }
}

// true: 表示此次复位是通过app程序收到iap升级指令
// false: 正常复位, 直接跳转
bool IAP_RAM_Check(void)
{
  uint32_t val = *(uint32_t*)IAP_MAGIC_ADDR;
  return val == IAP_MAGIC_VAL;
}

void IAP_Run(void)
{
  switch (state) {
  case IAP_SEND_ACK:{
    response(IAP_ACK_BYTE);
    state = IAP_WAIT_PACKAGE;
    break;
  }
  case IAP_WAIT_PACKAGE:{
    if(waitPackage()){
      state = IAP_WRITE_FLASH;
    }
    else{
      state = IAP_FINISH;
    }
    break;
  }
  case IAP_WRITE_FLASH:{
    writeFlash();
    state = IAP_SEND_ACK;
    break;
  }
  case IAP_FINISH:{
    // 清除标志后软件复位，自动重置所有外设，并跳转app
    ramClear();
    HAL_NVIC_SystemReset();
    return;
  }
  default:
    return;
  }
}

void IAP_JumpApp(void)
{
  uint32_t appStack = *(__IO uint32_t*)IAP_APP_ADDR;
  if ((appStack & 0x20000000) == 0) {
    errorHandler(IAP_ERROR_APPSTACK);
  }
  typedef void (*pFunction)(void);
  pFunction Jump_To_Application;

  uint32_t JumpAddress = *(__IO uint32_t*)(IAP_APP_ADDR + 4);
  Jump_To_Application = (pFunction)JumpAddress;

  __disable_irq();

  __set_MSP(appStack);

  Jump_To_Application();
}
