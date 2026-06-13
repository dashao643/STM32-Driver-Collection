#ifndef __PWM_IN_H__
#define __PWM_IN_H__

#include "tim.h"

// PWM输入模式测频率和占空比(双通道)
// 此定时器周期应大于被测PWM周期(PSC ARR大)
// 通道1配置为上升沿触发，通道2配置为下降沿触发(测高电平时间)

// 如果测量的pwm占空比为0或100，则测量值为最近的不为0或100的值

// #define PWM_IN_ENABLE

#define PWM_IN_INSTANCE       TIM1
#define PWM_IN_HANDLE         &htim1
#define PWM_IN_CHANNEL1       TIM_CHANNEL_1
#define PWM_IN_CHANNEL2       TIM_CHANNEL_2

#define PWM_IN_PSC            719

typedef struct {
  uint16_t periodCnt;        // 总周期计数
  uint16_t highCnt;          // 高电平计数
} PWM_IN_t;

void PWM_IN_Init(void);
uint32_t PWM_IN_GetFrequency(void);
uint8_t PWM_IN_GetDutyCycle(void);

#endif
