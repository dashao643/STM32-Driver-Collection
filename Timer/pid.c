#include "stm32f1xx_hal.h"
#include "pid.h"
#include "tb6612.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "led.h"

#define PRINTF_DEBUG

static PID_t pid = {0};

/**
 * @brief PID调控函数，主循环调用
 * 
 * @param type 定速控制或定位控制：PID_CONSTANT_SPEED_CONTROL / PID_CONSTANT_POSITION_CONTROL
 */
void PID_Task(PID_ControlType_e type)
{
  if(HAL_GetTick() - pid.timer < HALL_ENCODER_MEASURE_INTERVAL)
    return;

  pid.timer = HAL_GetTick();
 
  if(pid.isStart){
    if(type == PID_CONSTANT_SPEED_CONTROL)
      pid.actual = abs(HALL_Encoder_GetRPM());
    // else if(type == PID_CONSTANT_POSITION_CONTROL)
    //   pid.actual = abs(HALL_Encoder_GetAngle());
    
    pid.lastError = pid.thisError;
    pid.thisError = pid.target - pid.actual;

    pid.errorInter += pid.thisError;
    // 积分限幅
    if(pid.errorInter > 300) pid.errorInter = 300;
    if(pid.errorInter < -300) pid.errorInter = -300;


    float errorDiff = pid.thisError - pid.lastError;

    pid.out = PID_Kp * pid.thisError + PID_Ki * pid.errorInter + PID_Kd * errorDiff;

    TB6612_SetDuty(pid.out);

#ifdef PRINTF_DEBUG
    printf("actual=%d\n",(int)pid.actual);
    printf("thisError=%d\n",(int)pid.thisError);
    printf("lastError=%d\n",(int)pid.lastError);
    printf("errorInter=%d\n",(int)pid.errorInter);
    printf("out=%d\n",(int)pid.out);
#endif
  }
}

/**
 * @brief PID控制电机正转
 * 
 * @param target 目标转速或目标角度
 */
void PID_Forward(uint16_t target)
{
  pid.target = target;

  pid.errorInter = 0;
  pid.thisError = 0;
  TB6612_Forward(PID_BASE_DUTY);

  pid.isStart = true;
}

/**
 * @brief PID控制电机反转
 * 
 * @param target 目标转速或目标角度
 */
void PID_Reverse(uint16_t target)
{
  pid.target = target;

  pid.errorInter = 0;
  pid.thisError = 0;
  TB6612_Reverse(PID_BASE_DUTY);

  pid.isStart = true;
}

void PID_Stop(void)
{
  TB6612_Brake();

  pid.errorInter = 0;
  pid.thisError = 0;
  pid.isStart = false;
}
