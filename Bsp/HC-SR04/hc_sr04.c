#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"
#include "hc_sr04.h"
#include "general.h"

#include <stdint.h>
#include <stdbool.h>

// PIN_HIGH PIN_LOW
inline static void Trig_Set(GPIO_PinState pinState)
{
  HAL_GPIO_WritePin(HC_SR04_TRIG_GPIO_Port, HC_SR04_TRIG_Pin, pinState);
}

inline static GPIO_PinState Echo_Read(void)
{
  return HAL_GPIO_ReadPin(HC_SR04_ECHO_GPIO_Port, HC_SR04_ECHO_Pin);
}

/**
 * @brief 等待Echo引脚为指定状态
 * 
 * @param timer_us 返回等待时长(单位: 微秒)
 * @param pinState 指定等待的引脚状态
 * @return true 等待超时
 */
static bool waitEchoIsTimeout(uint16_t *timer_us, GPIO_PinState pinState)
{
  *timer_us = 0;

  while(Echo_Read() != pinState) {
    (*timer_us)++;
    Delay_Us(1);

    if((*timer_us) > HC_SR04_TIMEOUT_US)
      return true;
  }
  return false;
}

void HC_SR04_Init(void)
{
  Trig_Set(PIN_LOW);
}

/**
 * @brief 测量函数
 * 
 * @param distance_mm 返回测量到的距离(单位: 毫米)(4 - 200mm)
 * @return HAL_StatusTypeDef 
 *  @arg HAL_OK 测量成功
 *  @arg HAL_ERROR 等待高电平超时, 检查HC_SR04硬件
 *  @arg HAL_TIMEOUT 检测超时, 距离过远
 */
HAL_StatusTypeDef HC_SR04_Measure(uint16_t *distance_mm)
{
  uint16_t timer_us = 0;

  Trig_Set(PIN_HIGH);
  Delay_Us(HC_SR04_TRIGGER_US);
  Trig_Set(PIN_LOW);

  // 等待Echo变为高电平
  if(waitEchoIsTimeout(&timer_us, PIN_HIGH)) return HAL_ERROR;

  // 等待Echo变为低电平
  if(waitEchoIsTimeout(&timer_us, PIN_LOW)) return HAL_TIMEOUT;

  *distance_mm = (float)timer_us / HC_SR04_CAL_DISTANCE;

  //  = cm * 10;

  return HAL_OK;
}
