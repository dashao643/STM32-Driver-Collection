#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include "ds1302.h"
#include "general.h"
#include <stdint.h>


/*
低字节 低位先发
CE 高电平期间 读写
CLK 上升沿写 下降沿读
CE 初始必须为低 当CE被驱动为逻辑1状态时，SCLK必须为逻辑0
突发模式之前，和写操作之前 先清除写保护
*/

// PIN_HIGH / PIN_LOW
static inline void CE_Set(GPIO_PinState pinState)
{
  HAL_GPIO_WritePin(DS18B20_CE_GPIO_Port , DS18B20_CE_Pin, pinState);
}

static inline void CLK_Set(GPIO_PinState pinState)
{
  HAL_GPIO_WritePin(DS18B20_CLK_GPIO_Port , DS18B20_CLK_Pin, pinState);
}

static inline void DAT_SetPin(GPIO_PinState pinState)
{
  HAL_GPIO_WritePin(DS18B20_DAT_GPIO_Port , DS18B20_DAT_Pin, pinState);
}

// GPIO_MODE_INPUT / GPIO_MODE_OUTPUT_PP
static inline void DAT_SetMode(uint32_t mode)
{
  if(mode != GPIO_MODE_INPUT && mode != GPIO_MODE_OUTPUT_PP) return;

  GPIO_InitTypeDef gpio;
  gpio.Mode = mode;
  gpio.Pin = DS18B20_DAT_Pin;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_MEDIUM;

  HAL_GPIO_Init(DS18B20_DAT_GPIO_Port, &gpio);
  DAT_SetPin(PIN_LOW);
}

// 内部函数不操作CE,如果下一时钟是读操作，CLK不拉低
static void writeBit(uint8_t bit, bool isRead)
{
  if(bit != 0 && bit != 1) return;

  DAT_SetPin((GPIO_PinState)bit);
  CLK_Set(PIN_HIGH);
  Delay_Us(1);
  if(!isRead){
    CLK_Set(PIN_LOW);
  }
}

// 先传低位
static void writeByte(uint8_t byte, bool isRead)
{
  DAT_SetMode(GPIO_MODE_OUTPUT_PP);
  for(uint8_t i = 0; i < 8; i++){
    // 是读操作并且在最后一个字节，CLK不拉低
    writeBit(byte & 0x01, isRead && (i == 7));
    byte >>= 1;
  }
}

static uint8_t readBit(void)
{
  CLK_Set(PIN_HIGH);
  uint8_t bit = HAL_GPIO_ReadPin(DS18B20_DAT_GPIO_Port, DS18B20_DAT_Pin);
  CLK_Set(PIN_LOW);
  Delay_Us(1);

  return bit;
}

static uint8_t readByte(void)
{
  // for(uint8_t i = 0; i < 8; i++){
  //   // 是读操作并且在最后一个字节，CLK不拉低
  //   writeBit(byte & 0x01, isRead && (i == 7));
  //   byte >>= 1;
  // }
}

// DS1302_CMD_WRITE / DS1302_CMD_READ
void cmdSend(uint8_t rw, uint8_t cmd)
{
  if(rw != DS1302_CMD_WRITE && rw != DS1302_CMD_READ) return;

  writeByte(rw | cmd);
}



/*-----------------------------------------------------------------*/

void DS1302_Init(void)
{
  DAT_SetMode(GPIO_MODE_OUTPUT_PP);
  CE_Set(PIN_LOW);
  CLK_Set(PIN_LOW);
  DAT_SetPin(PIN_LOW);
}

void WriteEnable(void)
{
  CE_Set(PIN_HIGH);



  CE_Set(PIN_LOW);
}