#include "stm32f1xx_hal.h"
#include "tb6612.h"
#include <stdint.h>
#include <stdio.h>

// PSC:719 ARR:99 CCR:0-100

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

void TB6612_Forward(uint8_t duty)
{
  if(duty > TB6612_PWM_CCR_MAX_VALUE) 
    duty = TB6612_PWM_CCR_MAX_VALUE;
  if(duty < TB6612_PWM_CCR_MIN_VALUE)
    duty = TB6612_PWM_CCR_MIN_VALUE;

  TB6612_SetModel(FORWARD);
  __HAL_TIM_SET_COMPARE(TB6612_PWM_HANDLE, TB6612_PWM_CHANNEL, duty);
}

void TB6612_Reverse(uint8_t duty)
{
  if(duty > TB6612_PWM_CCR_MAX_VALUE) 
    duty = TB6612_PWM_CCR_MAX_VALUE;
  if(duty < TB6612_PWM_CCR_MIN_VALUE)
    duty = TB6612_PWM_CCR_MIN_VALUE;

  TB6612_SetModel(REVERSE);
  __HAL_TIM_SET_COMPARE(TB6612_PWM_HANDLE, TB6612_PWM_CHANNEL, duty);
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

uint16_t TB6612_SpeedAdd(uint8_t step)
{
  if(step < TB6612_PWM_CCR_MIN_VALUE) 
    step = TB6612_PWM_CCR_MIN_VALUE;

  uint16_t CCR = __HAL_TIM_GET_COMPARE(TB6612_PWM_HANDLE, TB6612_PWM_CHANNEL);
  
  CCR += step;
  if(CCR > TB6612_PWM_CCR_MAX_VALUE)
    CCR = TB6612_PWM_CCR_MAX_VALUE;

  __HAL_TIM_SET_COMPARE(TB6612_PWM_HANDLE, TB6612_PWM_CHANNEL, CCR);

  return CCR;
}

uint16_t TB6612_SpeedSub(uint8_t step)
{
  if(step < TB6612_PWM_CCR_MIN_VALUE) 
    step = TB6612_PWM_CCR_MIN_VALUE;

  uint16_t CCR = __HAL_TIM_GET_COMPARE(TB6612_PWM_HANDLE, TB6612_PWM_CHANNEL);
  
  // printf("step=%d\n",step);
  if(CCR <= step)
    CCR = TB6612_PWM_CCR_MIN_VALUE;
  else
    CCR -= step;

  // printf("sub CCR=%d\n",CCR);
  __HAL_TIM_SET_COMPARE(TB6612_PWM_HANDLE, TB6612_PWM_CHANNEL, CCR);

  return CCR;
}
