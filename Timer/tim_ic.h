#ifndef __TIM_IC_H__
#define __TIM_IC_H__

#include "tim.h"
#include <stdint.h>

// 输入捕获测PWM频率
// NVIC开启捕获比较中断
// ARR设置为最大65535
// 测量周期内CNT溢出情况未考虑

#define TIM_CLOCK_FREQUENCY   72000000U
#define TIM_IC_PSC            719

#define TIM_IC_INSTANCE       TIM1
#define TIM_IC_HANDLE         &htim1
#define TIM_IC_CHANNEL        TIM_CHANNEL_4

typedef enum {
  LAST_TIME = 0,
  THIS_TIME
} TIM_IC_Time_e;            // 1/2：记录赋值给上次cnt还是此次cnt

typedef struct {
  uint16_t lastCnt;
  uint16_t thisCnt;
  uint16_t difCnt;
  TIM_IC_Time_e time;          
} TIM_IC_t;

void TIM_IC_Init(void);
uint32_t TIM_IC_GetFrequency(void);

#endif
