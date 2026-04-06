#include "dht11.h"
#include "general.h"
#include "stm32f1xx_hal.h"

#include <stdio.h>
#include <string.h>

static DHT11_HandleTypeDef dht11;

static void DHT11_SetOutput(void);
static void DHT11_SetInput(void);
static bool DHT11_WaitState(GPIO_PinState pinState,const uint8_t timeout_us);
static bool DHT11_ReadResponse(void);
static uint8_t DHT11_ReadByte(void);
static bool DHT11_checkSum(void);

// DHT11_Pin 初始：输出模式，高电平

/**
 * @brief 设置为推挽输出模式
 * 
 */
static void DHT11_SetOutput(void)
{                        
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin = DHT11_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

  HAL_GPIO_Init(DHT11_GPIO_Port, &GPIO_InitStruct);
}

/**
 * @brief 设置为上拉输入模式
 * 
 */
static void DHT11_SetInput(void)
{                         
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin = DHT11_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

  HAL_GPIO_Init(DHT11_GPIO_Port, &GPIO_InitStruct);
}

/**
 * @brief 等待DHT11置电平
 * 
 * @param pinState 期望的总线电平
 * @param timeout_us 超时时间
 * @return true 成功收到DHT11的回应
 * @return false 超时时间内DHT11无回应
 */
static bool DHT11_WaitState(GPIO_PinState pinState,const uint8_t timeout_us)
{
  uint8_t curTime = 0;
  while(DHT11_PIN_READ() != pinState){
    curTime++;
    Delay_us(1);
    if (curTime > timeout_us) 
      return false;
  }
  return true;
}

/**
 * @brief 读取DHT11的回应
 * 
 * @return true 读取成功
 * @return false 读取失败，无回应
 */
static bool DHT11_ReadResponse(void)
{
  DHT11_SetInput();

  // 等待 DHT11 拉低（83us）
  if(!DHT11_WaitState(GPIO_PIN_RESET, 100))
    return false;

  // 等待 DHT11 拉高（87us）
  if(!DHT11_WaitState(GPIO_PIN_SET, 100))
    return false;

  return true;
}

/**
 * @brief 读取单总线的一个字节
 * 
 * @return uint8_t 字节数据
 */
static uint8_t DHT11_ReadByte(void)
{
  uint8_t aByte = 0x00;
  // 循环读8个比特位
  for(uint8_t i = 0; i < 8; i++){
    // 等待变为低电平
    if(!DHT11_WaitState(GPIO_PIN_RESET, 60)){
      // OLED_Show_String(2, 1, "data fail_1");
      return aByte;
    }
    // 等待变为高电平
    if(!DHT11_WaitState(GPIO_PIN_SET, 60)){
      // OLED_Show_String(2, 1, "data fail_2");
      return aByte;
    }
    Delay_us(40);
    // 延时以后读到高为bit1。低为bit0，直接跳过
    aByte <<= 1;
    if(DHT11_PIN_READ() == GPIO_PIN_SET)
      aByte |= 0x01;
  }
  return aByte;
}

static bool DHT11_checkSum(void)
{
  uint8_t checkSum = 0;
  for(uint8_t i = 0; i < 4; i++)
    checkSum += dht11.data_buf[i];

  if(checkSum != dht11.data_buf[4])
    return false;

  return true;
} 

void DHT11_Init(void)
{
  // 清空整个句柄
  memset(&dht11, 0, sizeof(DHT11_HandleTypeDef));
  dht11.state = DHT11_STATE_IDLE;
  dht11.last_read_tick = HAL_GetTick();
  dht11.humidity = 0x00;
  dht11.temperature = 0x00;
  dht11.data_valid = false;
  memset(dht11.data_buf,0,sizeof(dht11.data_buf));
}

void DHT11_Task(void)
{
  switch (dht11.state) {
    case DHT11_STATE_IDLE:{
      dht11.data_valid = false;
      if((HAL_GetTick() - dht11.last_read_tick) > DHT11_READ_INTERVAL_MS){
        dht11.last_read_tick = HAL_GetTick();
        dht11.state = DHT11_STATE_START_LOW;
      }
      break;
    }
    case DHT11_STATE_START_LOW:{
      DHT11_SetOutput();
      DHT11_PIN_LOW();
      dht11.start_tick = HAL_GetTick();
      dht11.state = DHT11_STATE_START_HIGH;
      break;
    }
    case DHT11_STATE_START_HIGH:{
      if ((HAL_GetTick() - dht11.start_tick) >= DHT11_Start_MS) {
        DHT11_PIN_HIGH();
        Delay_us(13);
        dht11.state = DHT11_STATE_WAIT_ACK;
      }
      break;
    }
    case DHT11_STATE_WAIT_ACK:{
      if(DHT11_ReadResponse())
        dht11.state = DHT11_STATE_READ_DATA;
      else{
        printf("dht11 no response\n");
        dht11.state = DHT11_STATE_IDLE;
      }
      break;
    }
    case DHT11_STATE_READ_DATA:{
      for(uint8_t i = 0; i < 5; i++)
        dht11.data_buf[i] = DHT11_ReadByte();
      dht11.state = DHT11_STATE_CHECK;
      break;
    }
    case DHT11_STATE_CHECK:{
      if(DHT11_checkSum()){
        dht11.humidity = dht11.data_buf[0];
        dht11.temperature = dht11.data_buf[2];
        dht11.data_valid = true;
      }
      else{
        printf("dht11 checkSum error\n");
      }
      dht11.state = DHT11_STATE_IDLE;
      break;
    }
    default:
      dht11.state = DHT11_STATE_IDLE;
      break;
  }
}

bool DHT11_IsDataValid(void)
{
  return dht11.data_valid;
}

uint8_t DHT11_GetHumidity(void)
{
  return dht11.humidity;
}
uint8_t DHT11_GetTemperature(void)
{
  return dht11.temperature;
}