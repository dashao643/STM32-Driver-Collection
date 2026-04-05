#include "pwm.h"
#include "main.h"
#include "oled.h"
#include "gpio.h"
#include "stm32_hal_legacy.h"
#include "stm32f1xx.h"

#include <stdbool.h>
#include <stdint.h>

static int8_t curCnt = 0;
static int8_t preCnt = 0;

static uint8_t getEncoderCnt(void);
static void setEncoderCnt(uint8_t cntVal);
static bool cntIsChange(void);
static void setLedDuty(void);

/**
 * @brief ARR设置为99,cnt数值直接为占空比
 * 
 */
void PWM_LED_Init(void)
{
  // 开启定时器时钟
  HAL_TIM_Base_Start(PWM_LED_HANDLE);
  // 开启PWM输出
  HAL_TIM_PWM_Start(PWM_LED_HANDLE, PWM_LED_CHANNEL);
#ifdef OLED
  OLED_ShowString(1, 1, "ledDuty=");
  OLED_ShowDecNumber(1, 9, getEncoderCnt(), 3);
#endif
}

void PWM_Encoder_Init(void)
{
  HAL_TIM_Base_Start(PWM_ENCODER_HANDLE);
  // 开启编码器输入模式
  HAL_TIM_Encoder_Start(PWM_ENCODER_HANDLE, TIM_CHANNEL_ALL);
}

void PWM_SERVO_Init(void)
{
  HAL_TIM_Base_Start(PWM_SERVO_HANDLE);
  HAL_TIM_PWM_Start(PWM_SERVO_HANDLE, PWM_SERVO_CHANNEL);
  PWM_SetServoAngle(90);
}

/********************* LED + 编码器 ***************** */

/**
 * @brief Encoder Cnt 数值保持在0-100
 * 
 * @return uint8_t cnt始终为正
 */
static uint8_t getEncoderCnt(void)
{
  int8_t resCnt = __HAL_TIM_GET_COUNTER(PWM_ENCODER_HANDLE);
  if(resCnt < 0)
    setEncoderCnt(0);
  if(resCnt > 100)
    setEncoderCnt(100);
  return resCnt;
}

static void setEncoderCnt(uint8_t cntVal)
{
  __HAL_TIM_SET_COUNTER(PWM_ENCODER_HANDLE, cntVal);
}

static bool cntIsChange(void)
{
  preCnt = curCnt;
  curCnt = getEncoderCnt();
  
  if(curCnt == preCnt)
    return false;
  return true;
}

static void setLedDuty(void)
{
  uint8_t ccr = getEncoderCnt();
  __HAL_TIM_SET_COMPARE(PWM_LED_HANDLE,PWM_LED_CHANNEL,ccr);
}

/**
 * @brief 检测旋转编码器的转动，设置LED灯亮度
 * 
 */
void PWM_LED_Scan(void)
{
  if(cntIsChange()){
    setLedDuty();
#ifdef OLED
    OLED_ShowDecNumber(1, 9, getEncoderCnt(), 3);
#endif
  }
}

/************************* 舵机 **********************/

/**
 * @brief SG90舵机，分辨率为9度
 * 
 * @param angle 0-180度,对应ccr 5-25
 */
void PWM_SetServoAngle(uint8_t angle)
{
  uint8_t ccrVal = angle / 9 + 5;
  __HAL_TIM_SET_COMPARE(PWM_SERVO_HANDLE,PWM_SERVO_CHANNEL,ccrVal);
}

void PWM_AddServoAngle(void)
{
  uint8_t curCcr = __HAL_TIM_GET_COMPARE(PWM_SERVO_HANDLE,PWM_SERVO_CHANNEL);
  if(curCcr < 25){
    curCcr += SERVO_ANGLE_STEP / 9;
    __HAL_TIM_SET_COMPARE(PWM_SERVO_HANDLE,PWM_SERVO_CHANNEL,curCcr);
  }
}

void PWM_SubServoAngle(void)
{
  int8_t curCcr = __HAL_TIM_GET_COMPARE(PWM_SERVO_HANDLE,PWM_SERVO_CHANNEL);
  if(curCcr > 5){
    curCcr -= SERVO_ANGLE_STEP / 9;
    __HAL_TIM_SET_COMPARE(PWM_SERVO_HANDLE,PWM_SERVO_CHANNEL,curCcr);
  }
#ifdef OLED
  OLED_ShowString(2, 1, "curCcr=");
  OLED_ShowDecNumber(2, 8, curCcr, 3);
#endif
}

/************************ 直流电机 **********************/
static PWM_Motor_State motor;

void PWM_Motor_Init(void)
{
  HAL_TIM_Base_Start(PWM_MOTOR_HANDLE);
  HAL_TIM_PWM_Start(PWM_MOTOR_HANDLE, PWM_MOTOR_CHANNEL);
  motor.isStart = false;
  motor.dir = forward;
  motor.speed = 60;
}

void PWM_Motor_StartAndStop(void)
{
  // motorCheck();
  if(motor.isStart){
    HAL_GPIO_WritePin(MOTOR_CRL1_GPIO_Port,MOTOR_CRL1_Pin, RESET);
    HAL_GPIO_WritePin(MOTOR_CRL2_GPIO_Port,MOTOR_CRL2_Pin, RESET);
    motor.isStart = false;
    return;
  }
  // 电机此时为关闭，根据设定的方向开启电机
  if(motor.dir == forward){
    HAL_GPIO_WritePin(MOTOR_CRL1_GPIO_Port,MOTOR_CRL1_Pin, SET);
  }
  else if(motor.dir == reverse){
    HAL_GPIO_WritePin(MOTOR_CRL2_GPIO_Port,MOTOR_CRL2_Pin, SET);
  }
  motor.isStart = true;
}

/**
 * @brief 电机方向反转
 * 
 */
void PWM_Motor_ReverseDir(void)
{
  // 此时为启动状态，改变实际电平
  if(motor.isStart){
    HAL_GPIO_TogglePin(MOTOR_CRL1_GPIO_Port,MOTOR_CRL1_Pin);
    HAL_GPIO_TogglePin(MOTOR_CRL2_GPIO_Port,MOTOR_CRL2_Pin);
  }
  if(motor.dir == forward)
    motor.dir = reverse;
  else
    motor.dir = forward;
}

void PWM_Motor_SpeedAdd(void)
{
  if(motor.speed < 100){
    motor.speed += MOTOR_SPEED_STEP;
    __HAL_TIM_SetCompare(PWM_MOTOR_HANDLE, PWM_MOTOR_CHANNEL,motor.speed);
  }
  // LED_RED_TOGGLE();
#ifdef OLED
  OLED_ShowString(2, 1, "curSpeed=");
  OLED_ShowDecNumber(2, 8, motor.speed, 3);
#endif
}

void PWM_Motor_SpeedSub(void)
{
  if(motor.speed > MOTOR_SPEED_STEP){
    motor.speed -= MOTOR_SPEED_STEP;
    __HAL_TIM_SetCompare(PWM_MOTOR_HANDLE, PWM_MOTOR_CHANNEL,motor.speed);
  }
  // LED_GREEN_TOGGLE();
#ifdef OLED
  OLED_ShowString(2, 1, "curSpeed=");
  OLED_ShowDecNumber(2, 8, motor.speed, 3);
#endif
}