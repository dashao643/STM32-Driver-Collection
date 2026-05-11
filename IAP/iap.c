#include "iap.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// 不使用外部FLASH，边接收边写入内部FLASH

/*
FLASH地址:        0x8000000 - 0x800FFFF 64KB
Bootloader地址:   0x8000000 - 0x8003FFF 16KB
App代码地址:      0x8004000 - 0x800FFFF 48KB
RAM地址：         0x20000000 - 0x20004FFF
*/

static IAP_e state = IAP_SEND_ACK;
static uint8_t rxBuf[IAP_RX_BUFF_MAXLENTH];
static uint8_t packageCnt = 0;

static void response(uint8_t byte)
{
#if IAP_MODEL == IAP_MODEL_FROM_UART
  HAL_UART_Transmit(IAP_HANDLE, &byte, 1, IAP_TX_TIMEOUT);
#endif 
}

static bool waitPackage(void)
{
#if IAP_MODEL == IAP_MODEL_FROM_UART
  if(HAL_UART_Receive(IAP_HANDLE, rxBuf, IAP_RX_BUFF_MAXLENTH, IAP_RX_TIMEOUT) == HAL_OK){
    return true;
  }
#endif 
  return false;
}

static bool writeFlash(void)
{
  if(packageCnt >=  48){
    response(IAP_ERROR_OVERFLOW);
    return false;
  }

  uint32_t eraseRes = 0;
  uint32_t addr = IAP_APP_ADDR + packageCnt * FLASH_PAGE_SIZE;
  FLASH_EraseInitTypeDef erase = {0};
  erase.TypeErase = FLASH_TYPEERASE_PAGES;
  erase.Banks = FLASH_BANK_1;
  erase.NbPages = 1;
  erase.PageAddress = addr;

  HAL_FLASH_Unlock();
  HAL_FLASHEx_Erase(&erase, &eraseRes);
  
  if(eraseRes != 0xFFFFFFFF){
    response(IAP_ERROR_ERASE);
    HAL_FLASH_Lock();
    return false;
  }
  // 一次写入2Byte,循环512次,一次跳2个数
  for(uint16_t i = 0; i < 1024; i+=2){
    uint16_t data = rxBuf[i] | (rxBuf[i + 1] << 8);
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, data);
    addr += 2;
  }
  memset(rxBuf, 0, IAP_RX_BUFF_MAXLENTH);
  HAL_FLASH_Lock();

  packageCnt++;

  return true;
}

bool IAP_RAM_Check(void)
{
  uint32_t val = *(uint32_t*)IAP_MAGIC_ADDR;
  return val == IAP_MAGIC_VAL;
}

void IAP_RAM_Clear(void)
{
  *(uint32_t*)IAP_MAGIC_ADDR = 0;
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
    if(!writeFlash()){
      while(1);
    }
    state = IAP_SEND_ACK;
    break;
  }
  case IAP_FINISH:{
    // response(packageCnt);
    // response(0xFF);

    // HAL_Delay(10);
    // while(__HAL_UART_GET_FLAG(IAP_HANDLE, UART_FLAG_TC) == RESET);
    // HAL_UART_Abort_IT(IAP_HANDLE);

    IAP_RAM_Clear();
    // HAL_NVIC_SystemReset();
    // IAP_DeInit();
    IAP_JumpApp();
    return;
  }
  default:
    return;
  }
}

void IAP_JumpApp(void)
{
  typedef void (*pFunction)(void);
  pFunction Jump_To_Application;

  uint32_t appStack = *(__IO uint32_t*)IAP_APP_ADDR;
  if ((appStack & 0x20000000) == 0) {
    while (1);
  }

  uint32_t JumpAddress = *(__IO uint32_t*)(IAP_APP_ADDR + 4);
  Jump_To_Application = (pFunction)JumpAddress;

  __disable_irq();
  SysTick->CTRL = 0;
  SysTick->LOAD = 0;
  SysTick->VAL  = 0;

  HAL_DeInit();

  __set_MSP(appStack);
  //   // 跳转前重新开中断，否则 app 的 SysTick IRQ 无法触发
  // __enable_irq();  

  Jump_To_Application();
}

void IAP_DeInit(void)
{
// #if IAP_MODEL == IAP_MODEL_FROM_UART 
//   HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);
// #endif

// #if IAP_MODEL == IAP_MODEL_FROM_EXT_FLASH 
//   HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5);
// #endif

//   // spi cs 
//   HAL_GPIO_DeInit(SPI1_CS1_GPIO_Port, SPI1_CS1_Pin);

//   __HAL_RCC_USART1_CLK_DISABLE();
//   __HAL_RCC_SPI1_CLK_DISABLE();
//   __HAL_RCC_GPIOA_CLK_DISABLE();
//   __HAL_RCC_GPIOB_CLK_DISABLE();
}