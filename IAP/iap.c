#include "iap.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/*
FLASH地址:        0x8000000 - 0x800FFFF 64KB
Bootloader地址:   0x8000000 - 0x8003FFF 16KB
App代码地址:      0x8004000 - 0x800FFFF 48KB
RAM地址：         0x20000000 - 0x20004FFF

此程序存储在flash起始区，初始上电时和从app跳转时执行此程序，
程序起始读取RAM中的一段数据，
若不符合，则关闭所有额外的外设，跳转到app程序地址正常执行app程序
若符合，则先发送ack，随后接收上位机数据，通过串口分包接收字节数据，写入内部falsh，写入完成后跳转到app程序地址，iap升级完成

上位机视角：发送握手信号，等待stm32回应。stm32app程序收到握手信号，跳转到bootloader程序，回应ack，
上位机收到ack后，再发送ack，之后开始传输数据，stm32接收数据

写入只能按半字：FLASH_TYPEPROGRAM_HALFWORD
HAL_StatusTypeDef HAL_FLASH_Program(uint32_t TypeProgram, uint32_t Address, uint64_t Data);
HAL_StatusTypeDef HAL_FLASH_Unlock(void);
HAL_StatusTypeDef HAL_FLASH_Lock(void);
HAL_StatusTypeDef  HAL_FLASHEx_Erase(FLASH_EraseInitTypeDef *pEraseInit, uint32_t *PageError);
*/

static IAP_e state = IAP_SEND_ACK;
static uint8_t rxBuf[IAP_RX_BUFF_MAXLENTH];
static uint8_t packageCnt = 0;

static void response(uint8_t byte)
{
#if IAP_MODEL == IAP_UART
  HAL_UART_Transmit(IAP_HANDLE, &byte, 1, IAP_TX_TIMEOUT);
#endif 
}

static bool waitPackage(void)
{
#if IAP_MODEL == IAP_UART
  if(HAL_UART_Receive(IAP_HANDLE, rxBuf, IAP_RX_BUFF_MAXLENTH, IAP_RX_TIMEOUT) == HAL_OK){
    return true;
  }
#endif 
  return false;
}

static void writeFlash(void)
{
  if(packageCnt >=  48){
    response(IAP_ERROR_OVERFLOW);
    return;
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
    return;
  }
  // 一次写入2Byte,循环512次,一次跳2个数
  for(uint8_t i = 0; i < 1024; i+=2){
    uint16_t data = rxBuf[i] | (rxBuf[i + 1] << 8);
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, data);
    addr += 2;
  }
  memset(rxBuf, 0, IAP_RX_BUFF_MAXLENTH);
  HAL_FLASH_Lock();

  packageCnt++;
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
    writeFlash();
    state = IAP_SEND_ACK;
    break;
  }
  case IAP_FINISH:{
    HAL_UART_Transmit(IAP_HANDLE, &packageCnt, 1, IAP_TX_TIMEOUT);
    IAP_DeInit();
    IAP_RAM_Clear();
    IAP_JumpApp();
    return;
  }
  default:
    return;
  }
}

void IAP_DeInit(void)
{
  // uart 
  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);
  // spi 
  HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5);
  // cs 
  HAL_GPIO_DeInit(SPI1_CS1_GPIO_Port, SPI1_CS1_Pin);
  // led
  HAL_GPIO_DeInit(LED_RED_GPIO_Port, LED_RED_Pin);
  HAL_GPIO_DeInit(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
  HAL_GPIO_DeInit(LED_BLUE_GPIO_Port, LED_BLUE_Pin);
}

void IAP_JumpApp(void)
{
  typedef void (*pFunction)(void);
  pFunction Jump_To_Application;
  uint32_t JumpAddress;

  __disable_irq();

  // 堆栈指针
  JumpAddress = *(__IO uint32_t*)(IAP_APP_ADDR + 4);
  Jump_To_Application = (pFunction)JumpAddress;

  // 设置堆栈
  __set_MSP(*(__IO uint32_t*)IAP_APP_ADDR);
  Jump_To_Application();
}