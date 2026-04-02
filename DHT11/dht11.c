#include "dht11.h"
#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#include "oled.h"
#include "gpio.h"
#include "myUsart.h"

static void DHT11_SetOutput(void);
static void DHT11_SetInput(void);
static void DHT11_Start(void);
static bool DHT11_WaitState(GPIO_PinState pinState,const uint8_t timeout_us);
static bool DHT11_ReadResponse(void);
static uint8_t DHT11_ReadByte(void);
static bool DHT11_checkSum(const uint8_t arr[]);

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
 * @brief DHT11发送起始信号
 * 
 */
static void DHT11_Start(void)
{
  DHT11_SetOutput();
  // 拉低18-30ms
  DHT11_LOW();
  HAL_Delay(20);
  // 拉高10-35us
  DHT11_HIGH();
  DHT11_Delay_us(13);
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
    DHT11_Delay_us(1);
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
static bool DHT11_ReadResponse()
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
    DHT11_Delay_us(40);
    // 延时以后读到高为bit1。低为bit0，直接跳过
    aByte <<= 1;
    if(DHT11_PIN_READ() == GPIO_PIN_SET)
      aByte |= 0x01;
  }
  return aByte;
}

/**
 * @brief 校验和
 * 
 * @param arr 包含固定四个字节的数组
 * @return true 校验成功
 * @return false 校验失败
 */
static bool DHT11_checkSum(const uint8_t arr[])
{
  uint8_t checkSum = 0;
  for(uint8_t j = 0; j < 4; j++)
    checkSum += arr[j];

  if(checkSum != arr[4])
    return false;

  return true;
} 

/**
 * @brief 读取DHT11的温湿度
 * 
 * @param temp 温度数据地址
 * @param humi 湿度数据地址
 */
void DHT11_ReadData(uint8_t *temp, uint8_t *humi)
{
  uint8_t dataArr[5] = {0};

  DHT11_Start();

  if(DHT11_ReadResponse() == false){
#ifdef OLED_DEBUG
    OLED_ShowString(2, 1, "no response");
    // LED_GREEN_TOGGLE();
#endif
    return;
  }
  // 获取数据
  for(uint8_t i = 0; i < 5; i++)
    dataArr[i] = DHT11_ReadByte();
  // 校验和
  if(DHT11_checkSum(dataArr) == false){
#ifdef OLED_DEBUG
    OLED_ShowString(2, 1, "checkSum error");
#endif

#ifdef LED_DEBUG
    LED_RED_TOGGLE();
#endif
    return;
  }
  *humi = dataArr[0];
  *temp = dataArr[2];
  
#ifdef USART1_DEBUG
  Usart1_Transmit(dataArr,sizeof(dataArr),100);
#endif
}

/**
 * @brief 微秒级延时，最长900us
 * 
 * @param us 微秒
 */
void DHT11_Delay_us(__IO uint32_t delay)
{
  int last, cur, val;
  int temp;

  while (delay != 0){
    temp = delay > 900 ? 900 : delay;
    last = SysTick->VAL;
    cur = last - CLOCK_FREQUENCY_MHZ * temp;
    if (cur >= 0){
      do
        val = SysTick->VAL;
      while ((val < last) && (val >= cur));
    }
    else{
      cur += CLOCK_FREQUENCY_MHZ * 1000;
      do
        val = SysTick->VAL;
      while ((val <= last) || (val > cur));
    }
    delay -= temp;
  }
}
