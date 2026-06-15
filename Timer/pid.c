#include "stm32f1xx_hal.h"
#include "pid.h"

/*
u:输出 e:偏差值 Kp:比例系数
u = Kp * e
Kp越大，系统响应越快，越快达到目标值
Kp过大会使系统产生较大的超调和振荡，导致系统的稳定性变差
仅有比例环节无法消除静态误差

p
int16_t error = target - actual;
int16_t output = Kp * error;   // Kp 从 0.3 开始试

i
integral += error;             // 每 10ms 累积一次
int16_t output = Kp * error + Ki * integral;

d
derivative = error - last_error;
int16_t output = Kp * error + Ki * integral + Kd * derivative;
*/