#ifndef __TIM_OUT_H__
#define __TIM_OUT_H__

#include "tim.h"

// 高级定时器输出指定数量PWM脉冲
// 重复计数器(RCR = N)：输出 N+1个PWM脉冲

#define PWM_OUT_SINGLE_PULSE            // 使用单脉冲模式

#define PWM_OUT_INSTANCE                TIM1
#define PWM_OUT_HANDLE                  &htim1
#define PWM_OUT_CHANNEL                 TIM_CHANNEL_4

#define PWM_OUT_RCR                     4    

// typedef struct {
//   uint32_t pulseOverflow;         // 累计溢出的脉冲数
//   uint16_t pulseInCycle;          // 一个溢出周期内的脉冲数
// } TIM_IN_t;

void PWM_OUT_Init(void);
void PWM_OUT_Start(void);
void PWM_OUT_UEV(void);

#endif
