#ifndef __TIM_IC_H__
#define __TIM_IC_H__

#include "tim.h"
#include <stdint.h>

// 单通道输入捕获测频率
// #define TIM_IC_ENABLE

#define TIM_IC_INSTANCE       TIM1
#define TIM_IC_HANDLE         &htim1
#define TIM_IC_CHANNEL        TIM_CHANNEL_4

#define TIM_IC_PSC            719

typedef enum {
  LAST_TIME = 0,             // 等待第一个上升沿
  THIS_TIME                  // 已捕获第一个上升沿，等待第二个
} TIM_IC_Time_e;

typedef struct {
  uint16_t difCnt;           // 两次捕获间定时器tick数
  TIM_IC_Time_e time;        // 当前测量状态
  uint16_t overflowTimes;    // 溢出次数
} TIM_IC_t;

void TIM_IC_Init(void);
uint32_t TIM_IC_GetFrequency(void);
void TIM_IC_UEV(void);

#endif
