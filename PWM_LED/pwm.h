#ifndef __PWM_H__
#define __PWM_H__

#include "tim.h"

#define PWM_LED_HANDLE        &htim2
#define PWM_LED_CHANNEL       TIM_CHANNEL_2

#define PWM_ENCODER_HANDLE    &htim1

#define OLED

void PWM_LED_Init(void);
void PWM_Encoder_Init(void);
void PWM_LED_Scan(void);

#endif
