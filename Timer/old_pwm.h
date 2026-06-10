#ifndef __PWM_H__
#define __PWM_H__

#include "tim.h"
#include <stdint.h>
#include <stdbool.h>

#define OLED

#define PWM_LED_HANDLE        &htim2
#define PWM_LED_CHANNEL       TIM_CHANNEL_2

#define PWM_ENCODER_HANDLE    &htim1

#define PWM_SERVO_HANDLE      &htim3
#define PWM_SERVO_CHANNEL     TIM_CHANNEL_1

#define PWM_MOTOR_HANDLE      &htim2
#define PWM_MOTOR_CHANNEL     TIM_CHANNEL_1

#define SERVO_ANGLE_STEP      45
#define MOTOR_SPEED_STEP      20

void PWM_LED_Init(void);
void PWM_Encoder_Init(void);
void PWM_SERVO_Init(void);

void PWM_LED_Scan(void);

void PWM_SetServoAngle(uint8_t angle);
void PWM_AddServoAngle(void);
void PWM_SubServoAngle(void);

/************************ 直流电机 **********************/
typedef enum{
  forward = 1,
  reverse = 0
} PWM_Motor_Dir;

typedef struct{
  bool isStart;
  PWM_Motor_Dir dir;
  uint8_t speed;
} PWM_Motor_State;

void PWM_Motor_Init(void);
void PWM_Motor_StartAndStop(void);
void PWM_Motor_ReverseDir(void);
void PWM_Motor_SpeedAdd(void);
void PWM_Motor_SpeedSub(void);

#endif
