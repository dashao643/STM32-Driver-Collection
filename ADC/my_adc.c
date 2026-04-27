#include "my_adc.h"
#include "adc.h"
#include "gpio.h"
#include "tim.h"
#include "stm32f1xx_hal.h"

#include <stdint.h>
#include <stdio.h>

static MY_ADC_t adc = {0};

// 返回值单位：毫伏
static int32_t calIrSensor(uint16_t adcRow);
static int32_t calTempESensor(uint16_t adcRow);
static int32_t calMQSensor(uint16_t adcRow);
static int32_t calTempISensor(uint16_t adcRow);
static int32_t calVREFSensor(uint16_t adcRow);

// 反射式红外传感器
static int32_t calIrSensor(uint16_t adcRaw)
{
  int32_t mv = (adcRaw * 3300) / 4095;

  return mv;
}

// 热敏电阻
static int32_t calTempESensor(uint16_t adcRaw)
{
  int32_t temp = 0;
  int32_t mv = (adcRaw * 3300) / 4095;
  
  if (mv >= 3300)
    mv = 3299;

  int32_t Rt = (10000L * mv) / (3300 - mv);

  if(Rt > 10000)
      temp = 2500 - ((Rt - 10000) / 35);  // 降温
  else
      temp = 2500 + ((10000 - Rt) / 30);  // 升温

  return temp;
}

// MQ-7 一氧化碳传感器 
// MQ-4 甲烷传感器
static int32_t calMQSensor(uint16_t adcRaw)
{
  int32_t mv = (adcRaw * 3300) / 4095;
  if (mv == 0)
    mv = 1;
  int32_t Rs = ((3300 - mv) * 1000L) / mv;

  return Rs / 10;
}

// 单片机内部温度传感器
static int32_t calTempISensor(uint16_t adcRaw)
{
  int32_t mv = (adcRaw * 3300) / 4095;
  int32_t temp = ((1430 - mv) * 100L) / 43 + 2500;
  return temp;
}

// 内部参考电压 VREFINT 测量
static int32_t calVREFSensor(uint16_t adcRaw)
{
  if (adcRaw == 0)
    adcRaw = 1;
  int32_t vdd = (1200L * 4095) / adcRaw;
  return vdd;
}

/**
 * @brief 硬件定时器触发ADC采集，当前为定时器更新事件触发
 * 
 */
void ADC_Init(void)
{
  HAL_TIM_Base_Start(ADC_TIM_HANDLE);
  // 若为输出比较触发，开启对应通道的PWM输出
#ifdef TIMER_CAPTURE_COMPARE_X_EVENT
  HAL_TIM_PWM_Start(ADC_TIM_HANDLE, ADC_TIM_CHANNEL);
#endif

  HAL_ADC_Start_DMA(ADC_HANDLE, (uint32_t*)adc.DMAbuf, ADC_NUMBER_OF_CONVERSION);
}

void ADC_Task(void)
{
  if(adc.readFlag){
    adc.readFlag = false;
    // 测试打印原始值
    for(int i=0; i<ADC_NUMBER_OF_CONVERSION; i++){
      printf("ch%d=%4d ", i+1, adc.DMAbuf[i]);
    }
    printf("\r\n");
    // 打印转化后数据
    for(int i=0; i<ADC_NUMBER_OF_CONVERSION; i++){
      printf("ch%d=%4d ", i+1, (int)ADC_GetValue(i));
    }
    printf("\r\n");
  }
  if(adc.infraredSensorFlag){
    adc.infraredSensorFlag = false;
    if((HAL_GetTick() - adc.infraredSensorTimer) > INFRARED_SENSOR_INTERVAL_MS){
      printf("Infrared Sensor DO Trigger\n");
      adc.infraredSensorTimer = HAL_GetTick();
    }
  }
}

void ADC_SetReadFlag(void)
{
  adc.readFlag = true;
}

int32_t ADC_GetValue(ADC_ChannelIndex_e ch)
{
  switch(ch){
    case ADC_CH_IR:      return calIrSensor(adc.DMAbuf[ch]);
    case ADC_CH_TEMP_E:  return calTempESensor(adc.DMAbuf[ch]);
    case ADC_CH_MQ7:     return calMQSensor(adc.DMAbuf[ch]);
    case ADC_CH_MQ4:     return calMQSensor(adc.DMAbuf[ch]);
    case ADC_CH_TEMP_I:  return calTempISensor(adc.DMAbuf[ch]);
    case ADC_CH_VREF:    return calVREFSensor(adc.DMAbuf[ch]);
    default:             return 0;
  }
}

void Infrared_Sensor_SetFlag(void)
{
  adc.infraredSensorFlag = true;
}

// 1s一次
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  if(hadc->Instance == ADC_INSTANCE){
    ADC_SetReadFlag();
  }
}

// 4位整数转成字符串显示(X.XXX)
void ADC_IntToString_1(int32_t val, char *str, uint8_t strlen)
{
  snprintf(str, strlen, "%ld.%03ld",val / 1000, val %1000);
}

// 4位整数转成字符串显示(XX.XX)
void ADC_IntToString_2(int32_t val, char *str, uint8_t strlen)
{
  snprintf(str, strlen, "%ld.%02ld",val / 100, val %100);
}