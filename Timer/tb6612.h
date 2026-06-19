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
#define TB6612_PWM_HANDLE               &htim2
#define TB6612_PWM_CHANNEL              TIM_CHANNEL_4

#define TB6612_PWM_CCR_MAX_VALUE        100
#define TB6612_PWM_CCR_MIN_VALUE        001

typedef enum{
  STOP = 0,
  FORWARD,
  REVERSE,
  BRAKE
} TB6612_Model_e;

void TB6612_Init(void);
void TB6612_Forward(uint8_t duty);
void TB6612_Reverse(uint8_t duty);
void TB6612_Stop(void);
void TB6612_Brake(void);

uint16_t TB6612_SpeedAdd(uint8_t step);
uint16_t TB6612_SpeedSub(uint8_t step);
void TB6612_SetDuty(uint8_t duty);

#endif
