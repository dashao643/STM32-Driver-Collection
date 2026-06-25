#include "stm32f1xx.h"
#include "stm32f1xx_hal.h"
#include "power.h"
#include "stm32f1xx_hal_pwr.h"
#include "tim.h"

#include <stdio.h>

void POWER_Init(void)
{
  if(__HAL_PWR_GET_FLAG(PWR_FLAG_WU) == SET){
    printf("被 WKUP RTC 事件唤醒");
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
  }
  if(__HAL_PWR_GET_FLAG(PWR_FLAG_SB) == SET){
    printf("从 standby 模式复位");
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
  }

}

void POWER_EnterSleep(void)
{
  // 暂停时钟
  HAL_TIM_Base_Stop_IT(&htim4);
  HAL_SuspendTick();

  // 进入睡眠模式
  HAL_PWR_EnterSLEEPMode(0, PWR_SLEEPENTRY_WFI);

  // 恢复时钟
  HAL_ResumeTick();
  HAL_TIM_Base_Start_IT(&htim4);
}

void POWER_EnterStop(void)
{
  // 暂停时钟
  HAL_TIM_Base_Stop_IT(&htim4);
  HAL_SuspendTick();

  // 清除外部中断,防止被阻止进入 Stop
  __HAL_GPIO_EXTI_CLEAR_FLAG(GPIO_PIN_All);

  // 进入停止模式
  HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);

  // 停止从此处唤醒后,默认使用HSI时钟,若要重新使用HSE,在main调用处后加 SystemClock_Config()

  // 恢复时钟
  HAL_ResumeTick();
  HAL_TIM_Base_Start_IT(&htim4);
}

void POWER_EnterStandby(void)
{
  HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
  HAL_PWR_EnterSTANDBYMode();
}
