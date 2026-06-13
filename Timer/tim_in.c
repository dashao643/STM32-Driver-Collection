#include "tim_in.h"

#include <stdint.h>
#include "led.h"
#include "stm32f1xx_hal_tim.h"

/*
定时器从模式：
            收到触发信号后                          典型场景
RESET:      计数器 CNT 立刻清零                     多路 PWM 严格同步
GATED:      触发信号 高电平 计数,低电平 暂停         只在主控允许时才统计脉冲
TRIGGER:    触发信号来一次,启动计数器                主从依次启动
EXTERNAL1:  把触发信号当成 CNT 的时钟心跳            外部脉冲计数

触发源:
内部触发(ITR0 - ITR3):             来自其他定时器的 TRGO(Trigger Output)
外部引脚滤波后(TI1FP1 / TI2FP2)：   来自本定时器的 CH1 或 CH2 引脚，经过输入滤波和边沿检测
外部时钟输入(ETRF)：              来自 ETR 引脚(External Trigger)，通常是一个外部时钟源
*/

static TIM_IN_t timIn = {0};

void TIM_IN_Init(void)
{
  HAL_TIM_Base_Start_IT(TIM_IN_HANDLE);
  // 不需要？
  HAL_TIM_IC_Start_IT(TIM_IN_HANDLE, TIM_IN_CHANNEL);
  // 不需要？

  timIn.pulseInCycle = 0;
  timIn.pulseOverflow = 0;
}

// 更新事件处理函数(CNT达到ARR,产生溢出)
void TIM_IN_CntOverflow(void)
{
  timIn.pulseOverflow += __HAL_TIM_GET_AUTORELOAD(TIM_IN_HANDLE);
}

// 读取32位脉冲数(从开机到现在的累计值)
uint32_t TIM_IN_GetPulseCnt(void)
{
  __disable_irq();
  timIn.pulseInCycle = __HAL_TIM_GET_COUNTER(TIM_IN_HANDLE);
  __enable_irq();

  return timIn.pulseInCycle + timIn.pulseOverflow;
}

// 试一下输入捕获用不用开中断
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if(htim->Instance == TIM_IC_INSTANCE){
    // TIM_IC_CntOverflow();
    LED_BLUE_TOGGLE();
  }
}
