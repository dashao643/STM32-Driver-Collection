#ifndef __MY_RTC_H__
#define __MY_RTC_H__

#include "rtc.h"
#include <stdbool.h>

// 支持同时含有时间寄存器和日历寄存器型号

/*
时钟源: LSE

Activate Clock Source:              true
Activate Calendar:                  true

Hour Format:                        HourFormat 24
Asynchronous Predivider value:      127
Synchronous Predivider value:       255
Data Format:                        Binary data format
*/

#define BKUP_FLAG_MAGIC     0xAA55
#define BKUP_REG_FLAG       RTC_BKP_DR1
// #define BKUP_REG_YEAR       RTC_BKP_DR2
// #define BKUP_REG_MONTH      RTC_BKP_DR3
// #define BKUP_REG_DATE       RTC_BKP_DR4

bool RTC_IsConfigured(void);
void RTC_GetDateTime(RTC_DateTypeDef *date, RTC_TimeTypeDef *time);
void RTC_SetDateTime(RTC_DateTypeDef *date, RTC_TimeTypeDef *time);
void RTC_ResetConfig(void);
void RTC_SetAlarm_IT(const RTC_TimeTypeDef *time);

#endif
