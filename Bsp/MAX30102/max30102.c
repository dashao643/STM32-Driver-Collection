#include "stm32f4xx_hal.h"
#include "max30102.h"
#include "max30102_def.h"
#include "i2c.h"
#include "stm32f4xx_hal_i2c.h"

#include <stdint.h>
#include <stdio.h>

// #define MAX_30102_INT_Pin GPIO_PIN_7
// #define MAX_30102_INT_GPIO_Port GPIOB
// #define MAX_30102_INT_EXTI_IRQn EXTI9_5_IRQn

// 从机地址最低位为读写位: 0 写, 1 读
/*
要读取位于FIFO_DATA之后的下一个寄存器，需要使用I2C写命令来改变读取指针的位置

SpO2:
MODE -> 0x03 
TEMP_EN -> 1
PPG_RDY-> 0
HR:
选择 red LED or the infrared LED channel 
MODE -> 0x02
PPG_RDY-> 0
*/

static void memRead(uint16_t memAddr, uint8_t *data)
{
  HAL_I2C_Mem_Read(MAX30102_HANDLE, MAX30102_SLAVE_ADDRESS, 
    memAddr, 1, data, 1, MAX30102_TX_TIMEOUT_MS);
}

static void memWrite(uint16_t memAddr, uint8_t data)
{
  HAL_I2C_Mem_Write(MAX30102_HANDLE, MAX30102_SLAVE_ADDRESS, 
    memAddr, 1, &data, 1, MAX30102_TX_TIMEOUT_MS);
}

void MAX30102_Init(void)
{
  // 使能所有中断
  memWrite(MAX30102_INTERRUPT_ENABLE_1, 0xF0);
  memWrite(MAX30102_INTERRUPT_ENABLE_2, 0x02);
	// memWrite(MAX30102_INTERRUPT_ENABLE_1,0xc0);
	// memWrite(MAX30102_INTERRUPT_ENABLE_2,0x00);

  // 清除读写指针
	memWrite(MAX30102_FIFO_WRITE_POINTER,0x00);
	memWrite(MAX30102_OVERFLOW_COUNTER,0x00);
	memWrite(MAX30102_FIFO_READ_POINTER,0x00);

  // sample avg = 32, fifo rollover=false, fifo almost full = 17(available=15)(中断发生时,FIFO中剩余空间)
	memWrite(MAX30102_FIFO_CONFIGURATION,0xEF);
  // 0x02 for Red only(测心率), 0x03 for SpO2 mode(测血氧), 0x07 multimode LED
	memWrite(MAX30102_MODE_CONFIGURATION,0x02);
  // SPO2_ADC range = 4096nA, SPO2 sample rate (100 Hz), LED pulseWidth (400uS)  
	memWrite(MAX30102_SPO2_CONFIGURATION,0x27);
  // Choose value for ~ 7mA for LED1
	memWrite(MAX30102_LED_PULSE_AMPLITUDE_1,0x24);   	
  // Choose value for ~ 7mA for LED2
	memWrite(MAX30102_LED_PULSE_AMPLITUDE_2,0x24);
  // Choose value for ~ 25mA for Pilot LED
	memWrite(MAX30102_PROXIMITY_MODE_LED_PULSE_AMPLITUDE,0x7F);

  MAX30102_ClearIRQ();
}

void MAX30102_Write_Test(void)
{

}

void readFIFO(void)
{
  uint8_t writePtr = 0;
  uint8_t readPtr = 0;
  uint8_t availableSamples = 0;
  uint8_t fifoData[6] = {0};

  // 获取写指针
  memRead(MAX30102_FIFO_WRITE_POINTER, &writePtr);
  // 获取读指针
  memRead(MAX30102_FIFO_READ_POINTER, &readPtr);
  availableSamples = writePtr - readPtr;
  printf("availableSamples=%d\n",availableSamples);

  // for(uint8_t i = 0; i < availableSamples; i++){
    HAL_I2C_Mem_Read(MAX30102_HANDLE, MAX30102_SLAVE_ADDRESS, 
    MAX30102_FIFO_DATA_REGISTER, 1, fifoData, 6, MAX30102_TX_TIMEOUT_MS);
  // }
  // 如果要重复读取,重新设置 Write FIFO RD_PTR;
  // if(availableSamples != 0){
    for(uint8_t i = 0; i < 6; i++){
      printf("fifoData=%d\n",fifoData[i]);
    }
  // }
}

void MAX30102_Read_Test(void)
{
  uint8_t data = 0;

  memRead(MAX30102_INTERRUPT_ENABLE_1, &data);
  printf("MAX30102_INTERRUPT_ENABLE_1=%d\n",data);
  memRead(MAX30102_INTERRUPT_ENABLE_2, &data);
  printf("MAX30102_INTERRUPT_ENABLE_1=%d\n",data);
  memRead(MAX30102_FIFO_DATA_REGISTER, &data);
  printf("MAX30102_FIFO_DATA_REGISTER=%d\n",data);
  memRead(MAX30102_DIE_TEMP_INTEGER, &data);
  printf("MAX30102_DIE_TEMP_INTEGER=%d\n",data);
}

void MAX30102_ClearIRQ(void)
{
  uint8_t data = 0;
  memRead(MAX30102_INTERRUPT_STATUS_1, &data);
  // printf("data=%d\n",data);
  memRead(MAX30102_INTERRUPT_STATUS_2, &data);
  // printf("data=%d\n",data);
}