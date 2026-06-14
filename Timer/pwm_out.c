#include "stm32f1xx_hal.h"
#include "pwm_out.h"

#include <stdint.h>
#include <stdio.h>

// 重复计数器(RCR)用来控制每N个计数器溢出(上溢或下溢)才产生一次更新事件(UEV)
// 重复计数器(RCR): N -> N + 1 个溢出触发一次更新

// 配置RCR + 单脉冲模式 或 RCR + UEV中断停止PWM

void PWM_OUT_Init(void)
{
#ifndef PWM_OUT_SINGLE_PULSE
  HAL_TIM_Base_Start(PWM_OUT_HANDLE);
#endif
#ifdef PWM_OUT_SINGLE_PULSE
  HAL_TIM_OnePulse_Start(PWM_OUT_HANDLE, PWM_OUT_CHANNEL);
#endif
}

void PWM_OUT_Start(void)
{
  __HAL_TIM_CLEAR_FLAG(PWM_OUT_HANDLE, TIM_FLAG_UPDATE);
  __HAL_TIM_ENABLE_IT(PWM_OUT_HANDLE, TIM_IT_UPDATE);
  HAL_TIM_PWM_Start(PWM_OUT_HANDLE, PWM_OUT_CHANNEL);
}

void PWM_OUT_UEV(void)
{
#ifndef PWM_OUT_SINGLE_PULSE
  HAL_TIM_PWM_Stop(PWM_OUT_HANDLE, PWM_OUT_CHANNEL);
#endif
}
