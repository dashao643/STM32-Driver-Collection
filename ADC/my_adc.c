#include "my_adc.h"
#include "gpio.h"
#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

  // uint16_t valFr = 0;
  // uint16_t valTemp = 0;
static bool infraredSensorFlag = false;
static uint32_t infraredSensorTimer = 0;
static uint16_t valFr = 0;

void MY_ADC_Task(void)
{
  if(infraredSensorFlag){
    infraredSensorFlag = false;
    if((HAL_GetTick() - infraredSensorTimer) > INFRARED_SENSOR_INTERVAL_MS){
      printf("Infrared Sensor DO Trigger\n");
      infraredSensorTimer = HAL_GetTick();
    }
  }
}

void Infrared_Sensor_SetFlag(void)
{
  infraredSensorFlag = true;
}

// 500ms一次
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  if(hadc->Instance == ADC1){
    LED_BLUE_TOGGLE();
    // valFr = HAL_ADC_GetValue(hadc);
    // printf("valFr=%d\n",valFr);
  }
}