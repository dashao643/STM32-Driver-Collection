#include "stm32f1xx_hal.h"
#include "ttp223.h"


// 未触摸时输出低, 触摸时输出高
#define TTP223_GPIO_Port     GPIOA
#define TTP223_Pin           GPIO_PIN_0

static bool trigger_;

void TTP223_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef gpio;

    gpio.Mode = GPIO_MODE_IT_RISING;
    gpio.Pull = GPIO_PULLDOWN;
    gpio.Pin = TTP223_Pin;

    HAL_GPIO_Init(TTP223_GPIO_Port, &gpio);

    HAL_NVIC_SetPriority(EXTI0_IRQn, 9, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);

    trigger_ = false;
}

// HAL_GPIO_EXTI_Callback 中调用
void TTP223_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == TTP223_Pin) {
		trigger_ = true;
	}
}

/**
 * @brief 主循环调用
 * 
 * @return true 按键触发(按下瞬间)
 * @return false 按键未触发
 */
bool TTP223_Task(void)
{
    if (trigger_) {
        trigger_ = false;
        return true;
    }
    return false;
}

void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(TTP223_Pin);
}