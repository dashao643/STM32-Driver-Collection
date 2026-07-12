#include "stm32f1xx_hal.h"
#include "system.h"

#include "led.h"
#include "dht11.h"
#include "oled.h"
#include "my_rtc.h"
#include "st7735.h"
#include "hc_sr04.h"

#include <stdint.h>
#include <stdio.h>

/********************* 选择RTC执行定时任务方式 *********************/
#define RTC_SYSTICK       // systick系统定时器方式
// #define RTC_TASK_SECONDS  // 秒中断方式
// #define RTC_TASK_TIMER    // timer定时器中断方式

static uint32_t dht11Timer;
static uint32_t rtcTimer;
static bool rtcFlag;
static uint32_t hcsr04Timer;

/********************* 系统任务初始化 *********************/
void System_Init(void)
{
  dht11Timer = HAL_GetTick();
  rtcTimer = HAL_GetTick();
  rtcFlag = false;
  hcsr04Timer = HAL_GetTick();

  OLED_ShowString(1, 1, "temp=");
  OLED_ShowString(1, 9, "humi=");

#ifdef RTC_TASK_SECONDS
  HAL_RTCEx_SetSecond_IT(&hrtc);
#endif
}

/********************* 系统任务 *********************/
void DHT11_Task(void)
{
  if(HAL_GetTick() - dht11Timer < 2000)  return;

  dht11Timer = HAL_GetTick();

  uint8_t temp = 0;
  uint8_t humi = 0;

  DHT11_Read(&temp, &humi);

  OLED_ShowDecNumber(1,6,temp,2);
  OLED_ShowDecNumber(1,14,humi,2);
}

void HC_SR04_Task(void)
{
  if(HAL_GetTick() - hcsr04Timer < 250)  return;

  hcsr04Timer = HAL_GetTick();

  uint16_t distance_mm = 0;
  uint8_t state = HC_SR04_Measure(&distance_mm);

  // printf("state=%d\n", state);
  // printf("distance_cm=%dmm\n", distance_mm);
}

void RTC_Task(void)
{
#ifdef RTC_SYSTICK
  if(HAL_GetTick() - rtcTimer < 1000)  return;
  rtcTimer = HAL_GetTick();
#elif defined RTC_TASK_SECONDS
  if(!rtcFlag) return;
  rtcFlag = false;
#endif
  RTC_TimeTypeDef time = {0};
  RTC_DateTypeDef date = {0};

  RTC_GetDateTime(&date, &time);

  char dateBuf[16] = {0};
  char timeBuf[16] = {0};

  // snprintf(dateBuf, sizeof(dateBuf), "%04d-%02d-%02d", 2000 + date.Year, date.Month,date.Date);
  // 省略显示XX年
  snprintf(dateBuf, sizeof(dateBuf), "%02d-%02d", date.Month, date.Date);
  snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d", time.Hours, time.Minutes, time.Seconds);

  // printf("test\n");
  OLED_ShowString(4, 1, dateBuf);
  OLED_ShowString(4, 7, timeBuf);

  ST7735_ShowString(1, 1, dateBuf, ST7735_WHITE);
  ST7735_ShowString(2, 1, timeBuf, ST7735_WHITE);
  // printf("test\n");
  // LED_RED_TOGGLE();
}

/********************* 系统中断回调函数 *********************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if(htim->Instance == TIM4){
  // LED_RED_TOGGLE();
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == KEY_8_Pin){
  // LED_RED_TOGGLE();
  }
}

#ifdef RTC_TASK_SECONDS
void HAL_RTCEx_RTCEventCallback(RTC_HandleTypeDef *hrtc)
{
  if(hrtc->Instance == RTC){
    // LED_BLUE_TOGGLE();
    system.rtcFlag = true;
  }
}
#endif

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
  if(hrtc->Instance == RTC){
    LED_RED_TOGGLE();
  }
}
