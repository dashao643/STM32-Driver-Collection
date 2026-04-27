#ifndef __MY_ADC_H
#define __MY_ADC_H

#include <stdbool.h>
#include <stdint.h>

// 选择定时器触发模式
#define TIMER_TRIGGER_OUT_EVENT         // 触发事件
// #define TIMER_CAPTURE_COMPARE_X_EVENT   // 输出比较触发

#define ADC_INSTANCE                    ADC1
#define ADC_HANDLE                      &hadc1
#define ADC_TIM_HANDLE                  &htim3
// #define ADC_TIM_CHANNEL                 TIM_CHANNEL_4
#define ADC_NUMBER_OF_CONVERSION        6                 // 6个ADC检测通道

#define INFRARED_SENSOR_INTERVAL_MS     200               // 数字信号触发间隔

typedef struct {
  uint16_t DMAbuf[ADC_NUMBER_OF_CONVERSION];
  bool readFlag;
  bool infraredSensorFlag;
  uint32_t infraredSensorTimer;
}MY_ADC_t;

typedef enum {
  ADC_CH_IR       = 0,      // 红外
  ADC_CH_TEMP_E   = 1,      // 外部温度
  ADC_CH_MQ7      = 2,      // 一氧化碳
  ADC_CH_MQ4      = 3,      // 甲烷
  ADC_CH_TEMP_I   = 4,      // 内部温度
  ADC_CH_VREF     = 5,      // 参考电压
} ADC_ChannelIndex_e;

void ADC_Init(void);
void ADC_Task(void);
void ADC_SetReadFlag(void);
int32_t ADC_GetValue(ADC_ChannelIndex_e ch);
void Infrared_Sensor_SetFlag(void);

void ADC_IntToString_1(int32_t val, char *str, uint8_t strlen);
void ADC_IntToString_2(int32_t val, char *str, uint8_t strlen);

#endif
