#ifndef __TB6612_H__
#define __TB6612_H__

#include "tim.h"

#include <stdint.h>
#include <stdbool.h>

/*
PWM输出0：       CCR >= (ARR + 1)
Mode:           PWM mode 1
Pulse:          CCR
preload         Enable
Fast Mode       Disable
CH Polarity     High
*/
#define TB6612_PWM_HANDLE       &htim2
#define TB6612_PWM_CHANNEL      TIM_CHANNEL_4

typedef enum{
  STOP = 0,
  FORWARD,
  REVERSE,
  BRAKE
} TB6612_Model_e;

void TB6612_Init(void);
void TB6612_Forward(void);
void TB6612_Reverse(void);
void TB6612_Stop(void);
void TB6612_Brake(void);

uint16_t TB6612_SpeedAdd(void);
uint16_t TB6612_SpeedSub(void);

#endif
