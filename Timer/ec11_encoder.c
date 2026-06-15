#include "stm32f1xx_hal.h"
#include "ec11_encoder.h"

#include <stdint.h>

void EC11_Encoder_Init(void)
{
  HAL_TIM_Base_Start(EC11_ENCODER_HANDLE);
  HAL_TIM_Encoder_Start(EC11_ENCODER_HANDLE, TIM_CHANNEL_ALL);
}

/**
 * @brief 获取EC11编码器的位置，最大值为转一圈
 * 
 * @return uint8_t 范围：0-100
 */
uint8_t EC11_Encoder_GetCnt(void)
{
  int16_t cnt = __HAL_TIM_GET_COUNTER(EC11_ENCODER_HANDLE);
  cnt = cnt / EC11_ENCODER_FREQUENCY_DOUBLING * 5;

  if(cnt < 0) {
    cnt = 0;
    __HAL_TIM_SET_COUNTER(EC11_ENCODER_HANDLE, cnt);
  }
  if(cnt > 100) {
    cnt = 100;
    __HAL_TIM_SET_COUNTER(EC11_ENCODER_HANDLE, cnt);
  }
  return cnt;
}
