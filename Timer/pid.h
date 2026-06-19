#ifndef __PID_H__
#define __PID_H__

#include "tim.h"
#include "hall_encoder.h"

#include <stdint.h>
#include <stdbool.h>

// 位置式PID
// PID调控周期与编码器测速周期保持一致

#define PID_Kp              0.2
#define PID_Ki              0.2
#define PID_Kd              0.0

#define PID_BASE_DUTY       30

typedef struct {
  uint16_t target;  // 目标值
  uint16_t actual;  // 实际值
  int16_t out;      // 输出值
  float thisError;  // 本次误差
  float lastError;  // 上次误差
  float errorInter; // 误差积分
  uint32_t timer;   // 调控间隔定时器
  bool isStart;
} PID_t;

typedef enum {
  PID_CONSTANT_SPEED_CONTROL = 0,   // 定速度控制
  PID_CONSTANT_POSITION_CONTROL     // 定位置控制
} PID_ControlType_e;

void PID_Task(PID_ControlType_e type);
void PID_Forward(uint16_t target);
void PID_Reverse(uint16_t target);
void PID_Stop(void);

#endif
