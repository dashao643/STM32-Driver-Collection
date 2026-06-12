#include "tim_ic.h"

#include <stdint.h>
#include <stdio.h>
#include <math.h>

static TIM_IC_t timIc = {0};

void TIM_IC_Init(void)
{
  HAL_TIM_Base_Start_IT(TIM_IC_HANDLE);
  HAL_TIM_IC_Start_IT(TIM_IC_HANDLE, TIM_IC_CHANNEL);

  timIc.time = LAST_TIME;
}

uint32_t TIM_IC_GetFrequency(void)
{
  // 获取分频系数: 
  // 0 0 0 0 -> 1
  // 0 1 0 0 -> 2
  // 1 0 0 0 -> 4
  // 1 1 0 0 -> 8
  uint8_t divByte = __HAL_TIM_GET_ICPRESCALER(TIM_IC_HANDLE, TIM_IC_CHANNEL);
  uint8_t div = pow(2, (divByte >> 2));

  uint32_t fre = TIM_CLOCK_FREQUENCY / (TIM_IC_PSC + 1) / timIc.difCnt * div;

  return fre;
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if(htim->Instance == TIM_IC_INSTANCE){
    if(timIc.time == LAST_TIME){
      timIc.lastCnt = __HAL_TIM_GET_COUNTER(TIM_IC_HANDLE);
      timIc.time = THIS_TIME;
    }
    else if(timIc.time == THIS_TIME){
      timIc.thisCnt = __HAL_TIM_GET_COUNTER(TIM_IC_HANDLE);
      timIc.time = LAST_TIME;
      timIc.difCnt = timIc.thisCnt - timIc.lastCnt;
    }
  }
}
