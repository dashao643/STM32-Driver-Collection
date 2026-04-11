#ifndef __MYRTC_H__
#define __MYRTC_H__

#include "stm32f1xx_hal.h"
#include <stdbool.h>

#define BKUP_REG1_FLAG  0xAA55

bool RTC_Check_BKUP_REG1(void);
void RTC_SetCurTimeDate(RTC_TimeTypeDef *sTime, RTC_DateTypeDef *sDate);
void RTC_GetCurTimeDate(RTC_TimeTypeDef *sTime, RTC_DateTypeDef *sDate);

void timeDateAdjust(void);
void RTC_SetShowFlag(void);
void RTC_OLED_ShowTask(void);

#endif

