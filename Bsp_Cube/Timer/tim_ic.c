#include "tim_ic.h"
#include "stm32f1xx_hal_tim.h"
#include "general.h"

#include <stdint.h>
#include <stdio.h>
#include <math.h>

// cnt为时间轴

static TIM_IC_t timIc = {0};

void TIM_IC_Init(void)
{
  HAL_TIM_Base_Start_IT(TIM_IC_HANDLE);
  HAL_TIM_IC_Start_IT(TIM_IC_HANDLE, TIM_IC_CHANNEL);

  timIc.time = LAST_TIME;
}

uint32_t TIM_IC_GetFrequency(void)
{
  // div: 0 0 0 0->1  0 1 0 0->2  1 0 0 0->4  1 1 0 0->8
  uint8_t divByte = __HAL_TIM_GET_ICPRESCALER(TIM_IC_HANDLE, TIM_IC_CHANNEL);
  uint8_t div = pow(2, (divByte >> 2));

  uint32_t fre = CLOCK_FREQUENCY_HZ / (TIM_IC_PSC + 1) / timIc.difCnt * div;

  return fre;
}

// HAL_TIM_PeriodElapsedCallback中调用
void TIM_IC_UEV(void)
{
  if (timIc.time == THIS_TIME)
    timIc.overflowTimes++;
}

#ifdef TIM_IC_ENABLE
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM_IC_INSTANCE) {
    if (timIc.time == LAST_TIME) {
      // 第一次上升沿：复位CNT，开始测量
      __HAL_TIM_SET_COUNTER(TIM_IC_HANDLE, 0);
      timIc.overflowTimes = 0;
      timIc.time = THIS_TIME;
    }
    else if (timIc.time == THIS_TIME) {
      // 第二次上升沿：完成一次周期测量
      uint32_t capVal = __HAL_TIM_GET_COMPARE(TIM_IC_HANDLE, TIM_CHANNEL_4);
      uint32_t arr = __HAL_TIM_GET_AUTORELOAD(TIM_IC_HANDLE) + 1;
      timIc.difCnt = timIc.overflowTimes * arr + capVal;
      timIc.time = LAST_TIME;
    }
  }
}
#endif
