#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include "dht11.h"
#include "general.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define PRINT_DEBUG

static DHT11_t dht11 = {0};

static inline void SDA_Set(GPIO_PinState pinState);
static inline GPIO_PinState SDA_Read(void);
static bool waitState(GPIO_PinState pinState, uint16_t timeout_us);
static uint8_t readByte(void);
static bool checkSum(void);

// PIN_HIGH / PIN_LOW
static inline void SDA_Set(GPIO_PinState pinState)
{
  HAL_GPIO_WritePin(DHT11_SDA_GPIO_Port , DHT11_SDA_Pin, pinState);
}

static inline GPIO_PinState SDA_Read(void)
{
  return HAL_GPIO_ReadPin(DHT11_SDA_GPIO_Port, DHT11_SDA_Pin);
}

static bool waitState(GPIO_PinState pinState, uint16_t timeout_us) 
{
  uint16_t curTime = 0;
  while (SDA_Read() != pinState) {
    curTime++;
    Delay_Us(1);
    if (curTime > timeout_us) {
#ifdef PRINT_DEBUG
      printf("dht11 wait timeout\n");
#endif
      return false;
    }
  }
  return true;
}

static uint8_t readByte(void) 
{
  uint8_t byte = 0x00;

  // 循环读8位
  for (uint8_t i = 0; i < 8; i++) {
    if (!waitState(PIN_LOW, 60))
      return byte;
    if (!waitState(PIN_HIGH, 60))
      return byte;

    Delay_Us(40);
    // 延时以后读到高为bit1。低为bit0，直接跳过
    byte <<= 1;
    if (SDA_Read() == PIN_HIGH)
      byte |= 0x01;
  }
  return byte;
}

static bool checkSum(void) 
{
  uint8_t checkSum = 0;
  for (uint8_t i = 0; i < 4; i++)
    checkSum += dht11.dataArr[i];

  if (checkSum != dht11.dataArr[4]) {
#ifdef PRINT_DEBUG
    printf("dht11 checkSum fail\n");
#endif
    return false;
  }

  return true;
}

/*-----------------------------------------------------------------*/

void DHT11_Init(void)
{
  // 默认输出高电平释放SDA
  SDA_Set(PIN_HIGH);

  dht11.timer = HAL_GetTick();
}

void DHT11_Read(uint8_t *temp, uint8_t *humi)
{
  if(HAL_GetTick() - dht11.timer < DHT11_MIN_INTERVAL_MS){
#ifdef PRINT_DEBUG
    printf("dht11 no arrival interval\n");
#endif
    return;
  }

  dht11.timer = HAL_GetTick();
  /********************* 起始信号 *********************/
  SDA_Set(PIN_LOW);
  HAL_Delay(DHT11_START_LOW_MS);
  SDA_Set(PIN_HIGH);
  Delay_Us(1);

  /********************* 等待回应 *********************/
  if(!waitState(PIN_LOW, 100))
    return;
  if(!waitState(PIN_HIGH, 100))
    return;

  /********************* 读取数据 *********************/
  for (uint8_t i = 0; i < 5; i++)
    dht11.dataArr[i] = readByte();

  /********************* 校验和 *********************/
  if (!checkSum()) return;

  *humi = dht11.dataArr[0];
  *temp = dht11.dataArr[2];
}
