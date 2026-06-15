#include "stm32f1xx_hal.h"
#include "hall_encoder.h"

#include <stdint.h>
#include <stdio.h>

// #define PRINTF_DEBUG

static HALL_ENCODER_t hall = {0};

void HALL_Encoder_Init(void)
{
  HAL_TIM_Base_Start(HALL_ENCODER_HANDLE);
  HAL_TIM_Encoder_Start(HALL_ENCODER_HANDLE, TIM_CHANNEL_ALL);

  hall.timer = HAL_GetTick();
  hall.lastCnt = __HAL_TIM_GET_COUNTER(HALL_ENCODER_HANDLE);
}

void HALL_Encoder_Task(void)
{
  uint32_t elapsed = HAL_GetTick() - hall.timer;

  if(elapsed < HALL_ENCODER_MEASURE_INTERVAL)
    return;

  hall.timer = HAL_GetTick();

  // 差值法读取脉冲，不清零计数器(避免读-清间隙丢脉冲)
  int16_t curCnt = __HAL_TIM_GET_COUNTER(HALL_ENCODER_HANDLE);
  int16_t deltaCnt = curCnt - hall.lastCnt;
  hall.lastCnt = curCnt;

#ifdef PRINTF_DEBUG
  printf("delta=%d, elapsed=%dms\n", deltaCnt, elapsed);
#endif
  // 用实际采样时间计算转速
  // 合并所有除数做单次除法，减少整数截断误差
  uint32_t totalDiv = (uint32_t)HALL_ENCODER_FREQUENCY_DOUBLING * HALL_ENCODER_PRECISION
                      * HALL_ENCODER_REDUCTION_RATIO * elapsed;

  hall.rpm = deltaCnt * 1000 * 60 / totalDiv;
#ifdef PRINTF_DEBUG
  printf("rpm=%d\n",hall.rpm);
#endif
}

/**
 * @brief 获取电机转速(单位：转/分)
 * 
 * @return int16_t -32768 ~ 32767，负值为反转
 */
int16_t HALL_Encoder_GetRPM(void)
{
  return hall.rpm;
}
