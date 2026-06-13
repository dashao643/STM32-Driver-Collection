#ifndef __TIM_IN_H__
#define __TIM_IN_H__

#include "tim.h"

// 定时器从模式测脉冲数
// 触发源选择TI1FP1 / TI2FP2
// PSC设置为0：1分频，一个脉冲CNT加1
// ARR设置为最大：减少溢出次数

#define TIM_IC_INSTANCE               TIM1
#define TIM_IN_HANDLE                 &htim1
#define TIM_IN_CHANNEL                TIM_CHANNEL_1

// #define TIM_IN_SAMPLE_INTERVAL_MS     100  // 采样间隔

typedef struct {
  uint32_t pulseOverflow;         // 累计溢出的脉冲数
  uint16_t pulseInCycle;          // 一个溢出周期内的脉冲数
} TIM_IN_t;

void TIM_IN_Init(void);
void TIM_IN_CntOverflow(void);
uint32_t TIM_IN_GetPulseCnt(void);

#endif
