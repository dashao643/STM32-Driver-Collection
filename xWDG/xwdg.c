#include "stm32f1xx_hal.h"
#include "xwdg.h"

static uint32_t iwdgTimer = 0;

/**
 * @brief 初始化喂狗间隔定时器，在初始化函数最后调用
 * 
 */
void IWDG_Init(void)
{
  // 当前间隔：(64 * 1875) / 40000 = 3s

  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_64;
  hiwdg.Init.Reload = 1874;

  HAL_IWDG_Init(&hiwdg);

  iwdgTimer = HAL_GetTick();
}

void IWDG_Refresh(uint16_t intervalMs)
{
  if((HAL_GetTick() - iwdgTimer) > intervalMs){
    iwdgTimer = HAL_GetTick();
    HAL_IWDG_Refresh(&hiwdg);
  } 
}
