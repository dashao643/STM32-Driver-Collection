#ifndef __EC11_ENCODER_H__
#define __EC11_ENCODER_H__

#include "tim.h"

#include <stdint.h>
#include <stdbool.h>

// EC11编码器旋钮，用于开关调节
// 逆时针CNT加，顺时针CNT减

// 外部信号为时钟源，PSC无效，配置为0
// ARR配置为65535，减小溢出
// Encoder Mode 配置为 TI1 and TI2(四倍频)

#define EC11_ENCODER_INSTANCE              TIM4
#define EC11_ENCODER_HANDLE                &htim4
#define EC11_ENCODER_CHANNEL1              TIM_CHANNEL_1
#define EC11_ENCODER_CHANNEL2              TIM_CHANNEL_2
#define EC11_ENCODER_FREQUENCY_DOUBLING    4 		          // 倍频数

#define EC11_ENCODER_PRECISION             20 	          // 编码器精度：每转一圈输出的脉冲数
#define EC11_ENCODER_REDUCTION_RATIO       01			        // 减速比

void EC11_Encoder_Init(void);
uint8_t EC11_Encoder_GetCnt(void);

#endif
