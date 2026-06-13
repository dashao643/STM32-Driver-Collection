#include "pwm_in.h"
#include "general.h"

#include <stdint.h>
#include <stdio.h>

static PWM_IN_t pwmIn = {0};

// cnt为时间轴
// 通道1捕获上升沿时，硬件自动把 CNT 复位成 0，并锁存到 CCR1

void PWM_IN_Init(void)
{
  HAL_TIM_Base_Start_IT(PWM_IN_HANDLE);

  HAL_TIM_IC_Start_IT(PWM_IN_HANDLE, PWM_IN_CHANNEL1);
  HAL_TIM_IC_Start_IT(PWM_IN_HANDLE, PWM_IN_CHANNEL2);
}

uint32_t PWM_IN_GetFrequency(void)
{
  uint32_t fre = CLOCK_FREQUENCY_HZ / (PWM_IN_PSC + 1) / (pwmIn.periodCnt + 1);

  // printf("periodCnt=%d\n",pwmIn.periodCnt);

  return fre;
}

// 0-100
uint8_t PWM_IN_GetDutyCycle(void)
{
  uint8_t duty = 0;

  // printf("highCnt=%d\n",pwmIn.highCnt);
  // printf("periodCnt=%d\n",pwmIn.periodCnt);

  duty = (pwmIn.highCnt + 1) * 100 / (pwmIn.periodCnt + 1);

  return duty;
}

#ifdef PWM_IN_ENABLE
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if(htim->Instance == PWM_IN_INSTANCE){
    pwmIn.periodCnt = __HAL_TIM_GET_COMPARE(PWM_IN_HANDLE, PWM_IN_CHANNEL1);
    pwmIn.highCnt = __HAL_TIM_GET_COMPARE(PWM_IN_HANDLE, PWM_IN_CHANNEL2);
  }
}
#endif
