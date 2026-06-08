#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_wwdg.h"
#include "xwdg.h"

#include <stdint.h>
#include <stdio.h>

static uint32_t iwdgTimer = 0;
static uint32_t wwdgTimer = 0;
/********************* IWDG *********************/

// 初始化独立看门狗定时器，在用户初始化函数最后调用
void IWDG_Init(void)
{
  // 当前间隔：(64 * 1875) / 40000 = 3s

  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_64;
  hiwdg.Init.Reload = 1874;

  HAL_IWDG_Init(&hiwdg);

  iwdgTimer = HAL_GetTick();
}

/**
 * @brief 喂狗函数
 * 
 * @param intervalMs 喂狗间隔(单位：ms)
 */
void IWDG_Refresh(uint16_t intervalMs)
{
  if((HAL_GetTick() - iwdgTimer) > intervalMs){
    iwdgTimer = HAL_GetTick();
    HAL_IWDG_Refresh(&hiwdg);
  } 
}

/********************* WWDG *********************/
void WWDG_Init(void)
{
  // 需要在Counter的值在Window值与0x40(64)之间刷新，0x40(64)->0x3F(63)时产生复位
  // 当前最短间隔：(4096 * 8) * (127 - 100 + 1) / 36MHz = 25.46ms 28
  // 当前最长间隔：(4096 * 8) * (127 - 64 + 1) / 36MHz = 58.25ms  64
  hwwdg.Instance = WWDG;
  hwwdg.Init.Prescaler = WWDG_PRESCALER_8;
  hwwdg.Init.Window = 127;
  hwwdg.Init.Counter = 127;
  hwwdg.Init.EWIMode = WWDG_EWI_DISABLE;

  HAL_WWDG_Init(&hwwdg);

  wwdgTimer = HAL_GetTick();
}

void WWDG_Refresh(uint16_t intervalMs)
{
  if((HAL_GetTick() - wwdgTimer) > intervalMs){
    wwdgTimer = HAL_GetTick();
    // printf("dashao\n");
    uint8_t state = HAL_WWDG_Refresh(&hwwdg);
    // printf("state=%d\n",state);
  } 
  
}

// EWI中断回调函数，在复位的前一个时钟触发
void HAL_WWDG_EarlyWakeupCallback(WWDG_HandleTypeDef *hwwdg)
{

}
