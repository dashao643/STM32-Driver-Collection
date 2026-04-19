#ifndef __MYRTC_H__
#define __MYRTC_H__

#include "stm32f1xx_hal.h"
#include <stdbool.h>

#define BKUP_FLAG_MAGIC     0xAA55

#define BKUP_REG_FLAG     RTC_BKP_DR1
#define BKUP_REG_YEAR     RTC_BKP_DR2
#define BKUP_REG_MONTH    RTC_BKP_DR3
#define BKUP_REG_DATE     RTC_BKP_DR4

bool RTC_CheckBkpReg1(void);
void RTC_SetAlarm(void);
void RTC_GetCurTimeDate(RTC_TimeTypeDef *sTime, RTC_DateTypeDef *sDate);

// RTC全局中断和Alarm中断同时开启，会死锁，问题待解决
void RTC_TimeDateAdjust(void);
void RTC_SetShowFlag(void);
void RTC_OLED_ShowTask(void);
void RTC_ClearRegFlag(void);

#endif

