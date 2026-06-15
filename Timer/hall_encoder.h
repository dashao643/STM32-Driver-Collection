#ifndef __HALL_ENCODER_H__
#define __HALL_ENCODER_H__

#include "tim.h"

#include <stdint.h>
#include <stdbool.h>

// 霍尔编码器，用于电机测速

// 外部信号为时钟源，PSC无效，配置为0
// ARR配置为65535，减小溢出
// Encoder Mode 配置为 TI1 and TI2(四倍频)

#define HALL_ENCODER_INSTANCE              TIM4
#define HALL_ENCODER_HANDLE                &htim4
#define HALL_ENCODER_CHANNEL1              TIM_CHANNEL_1
#define HALL_ENCODER_CHANNEL2              TIM_CHANNEL_2

#define HALL_ENCODER_FREQUENCY_DOUBLING    4 		          // 倍频数
#define HALL_ENCODER_PRECISION             13 	          // 编码器精度：每转一圈输出的脉冲数
#define HALL_ENCODER_REDUCTION_RATIO       30			        // 减速比

#define HALL_ENCODER_MEASURE_INTERVAL      100            // 测量电机转速间隔(单位：毫秒)

typedef struct {
  int16_t rpm;
  uint32_t timer;
  uint32_t lastCnt;
} HALL_ENCODER_t;

void HALL_Encoder_Init(void);
void HALL_Encoder_Task(void);
int16_t HALL_Encoder_GetRPM(void);

#endif
