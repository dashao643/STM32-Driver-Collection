#ifndef __ENCODER_H__
#define __ENCODER_H__

#include "tim.h"

#include <stdint.h>
#include <stdbool.h>

// 外部信号为时钟源，PSC无效，配置为0
// ARR配置为65535，减小溢出
// A相在前视为正转，B相在前视为反转

#define ENCODER_INSTANCE        TIM4
#define ENCODER_HANDLE          &htim4
#define ENCODER_CHANNEL1        TIM_CHANNEL_1
#define ENCODER_CHANNEL2        TIM_CHANNEL_2

// 编码型号：EC11
// 顺时针A相在前，逆时针B相在前
// 转一圈，20个脉冲
// #define EncoderMultiples   4.0 		//编码器倍频数
#define Encoder_precision  13.0 	//编码器精度 13线：每转一圈输出的脉冲数
// #define Reduction_Ratio  30.0			//减速比30
#define Perimeter  210.4867 			//周长，单位mm

void Encoder_Init(void);



#endif
