#include "pwm.h"
#include "key.h"
#include "oled.h"

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
  HAL_TIM_Base_Start_IT(PWM_ENCODER_HANDLE);
  // 开启编码器输入中断，固定为通道1和通道2
  HAL_TIM_Encoder_Start_IT(PWM_ENCODER_HANDLE, TIM_CHANNEL_1);
  HAL_TIM_Encoder_Start_IT(PWM_ENCODER_HANDLE, TIM_CHANNEL_2);
}

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
  __HAL_TIM_SET_COMPARE(PWM_LED_HANDLE,TIM_CHANNEL_2,ccr);
}

void PWM_LED_Scan(void)
{
  if(cntIsChange()){
    setLedDuty();
#ifdef OLED
    OLED_ShowDecNumber(1, 9, getEncoderCnt(), 3);
#endif
  }
}
