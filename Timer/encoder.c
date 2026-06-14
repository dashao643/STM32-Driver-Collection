#include "stm32f1xx_hal.h"
#include "encoder.h"
#include <stdint.h>

// cnt为位置轴

void Encoder_Init(void)
{
  HAL_TIM_Base_Start(ENCODER_HANDLE);
  // 开启编码器输入模式
  HAL_TIM_Encoder_Start(ENCODER_HANDLE, TIM_CHANNEL_ALL);
}
