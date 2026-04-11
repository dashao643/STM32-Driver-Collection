#include "myRtc.h"
#include "rtc.h"

#include "stdio.h"
#include "oled.h"

static bool showFlag = false;

static RTC_TimeTypeDef rtcTime = {0};
static RTC_DateTypeDef rtcDate = {0};

/**
 * @brief 检查BKP寄存器1（16位）是否已置标志
 * 
 * @return true 已置位
 * @return false 未置位
 */
bool RTC_Check_BKUP_REG1(void)
{
  if(HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) == BKUP_REG1_FLAG)
    return true;
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, BKUP_REG1_FLAG);

  return false;
}

void RTC_SetCurTimeDate(RTC_TimeTypeDef *sTime, RTC_DateTypeDef *sDate)
{
  HAL_RTC_SetTime(&hrtc, sTime, RTC_FORMAT_BIN);
  HAL_RTC_SetDate(&hrtc, sDate, RTC_FORMAT_BIN);
}

void RTC_GetCurTimeDate(RTC_TimeTypeDef *sTime, RTC_DateTypeDef *sDate)
{
  HAL_RTC_GetTime(&hrtc,sTime,RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc,sDate,RTC_FORMAT_BIN);
}

/***********************************************/
// 先用手动校准
void timeDateAdjust(void)
{
  rtcTime.Hours = 23;
  rtcTime.Minutes = 59;
  rtcTime.Seconds = 50;
  rtcDate.Year = 26;
  rtcDate.Month = 4;
  rtcDate.Date = 11;
  rtcDate.WeekDay = 6;
  RTC_SetCurTimeDate(&rtcTime, &rtcDate);
}

void RTC_SetShowFlag(void)
{
  showFlag = true;
}

void RTC_OLED_ShowTask(void)
{
  // if(showFlag){
    showFlag = false;
    RTC_GetCurTimeDate(&rtcTime, &rtcDate);

    char dateBuf[16] = {0};
    char timeBuf[16] = {0};

    snprintf(dateBuf, sizeof(dateBuf), "%04d-%02d-%02d", 2000 + rtcDate.Year, rtcDate.Month,rtcDate.Date);
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d", rtcTime.Hours, rtcTime.Minutes,rtcTime.Seconds);

    OLED_ShowString(1, 1, dateBuf);
    OLED_ShowString(2, 1, timeBuf);
  // }
}