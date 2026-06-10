#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include "tb6612.h"
#include "general.h"
#include "stm32f1xx_hal_tim.h"
#include <stdint.h>

// PSC:719 ARR:99 CCR:0-100
#define CCR_MAX_VALUE         100
#define CCR_DEFAULT_VALUE     60
#define CCR_SINGLE_STEP       20

static void TB6612_SetModel(TB6612_Model_e model);

static void TB6612_SetModel(TB6612_Model_e model)
{
  if(model == STOP){
    HAL_GPIO_WritePin(TB6612_IN1_GPIO_Port, TB6612_IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(TB6612_IN2_GPIO_Port, TB6612_IN2_Pin, GPIO_PIN_RESET);
  }
  else if(model == FORWARD){
    HAL_GPIO_WritePin(TB6612_IN1_GPIO_Port, TB6612_IN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(TB6612_IN2_GPIO_Port, TB6612_IN2_Pin, GPIO_PIN_RESET);
  }
  else if(model == REVERSE){
    HAL_GPIO_WritePin(TB6612_IN1_GPIO_Port, TB6612_IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(TB6612_IN2_GPIO_Port, TB6612_IN2_Pin, GPIO_PIN_SET);
  }
  else if(model == BRAKE){
    HAL_GPIO_WritePin(TB6612_IN1_GPIO_Port, TB6612_IN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(TB6612_IN2_GPIO_Port, TB6612_IN2_Pin, GPIO_PIN_SET);
    // 制动后回到停止模式，输出引脚高阻态
    HAL_Delay(1000);
    HAL_GPIO_WritePin(TB6612_IN1_GPIO_Port, TB6612_IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(TB6612_IN2_GPIO_Port, TB6612_IN2_Pin, GPIO_PIN_RESET);
  }
}

/*-----------------------------------------------------------------*/

void TB6612_Init(void)
{
  TB6612_SetModel(STOP);
  __HAL_TIM_SET_COMPARE(TB6612_PWM_HANDLE, TB6612_PWM_CHANNEL, 0);

  HAL_TIM_Base_Start(TB6612_PWM_HANDLE);
  HAL_TIM_PWM_Start(TB6612_PWM_HANDLE, TB6612_PWM_CHANNEL);
}

void TB6612_Forward(void)
{
  TB6612_SetModel(FORWARD);
  __HAL_TIM_SET_COMPARE(TB6612_PWM_HANDLE, TB6612_PWM_CHANNEL, CCR_DEFAULT_VALUE);
}

void TB6612_Reverse(void)
{
  TB6612_SetModel(REVERSE);
  __HAL_TIM_SET_COMPARE(TB6612_PWM_HANDLE, TB6612_PWM_CHANNEL, CCR_DEFAULT_VALUE);
}

void TB6612_Stop(void)
{
  TB6612_SetModel(STOP);
  __HAL_TIM_SET_COMPARE(TB6612_PWM_HANDLE, TB6612_PWM_CHANNEL, 0);
}

void TB6612_Brake(void)
{
  TB6612_SetModel(BRAKE);
  __HAL_TIM_SET_COMPARE(TB6612_PWM_HANDLE, TB6612_PWM_CHANNEL, 0);
}

uint16_t TB6612_SpeedAdd(void)
{
  uint16_t CCR = __HAL_TIM_GET_COMPARE(TB6612_PWM_HANDLE, TB6612_PWM_CHANNEL);
  
  if(CCR < CCR_MAX_VALUE)
    CCR += CCR_SINGLE_STEP;

  __HAL_TIM_SET_COMPARE(TB6612_PWM_HANDLE, TB6612_PWM_CHANNEL, CCR);

  return CCR;
}

uint16_t TB6612_SpeedSub(void)
{
  uint16_t CCR = __HAL_TIM_GET_COMPARE(TB6612_PWM_HANDLE, TB6612_PWM_CHANNEL);
  
  if(CCR > CCR_SINGLE_STEP)
    CCR -= CCR_SINGLE_STEP;

  __HAL_TIM_SET_COMPARE(TB6612_PWM_HANDLE, TB6612_PWM_CHANNEL, CCR);

  return CCR;
}