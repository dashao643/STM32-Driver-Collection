#include "myRtc.h"
#include "rtc.h"

#include "stdio.h"
#include "oled.h"
#include "gpio.h"

static bool showFlag = false;

static RTC_TimeTypeDef rtcTime = {0};
static RTC_DateTypeDef rtcDate = {0};

/**
 * @brief 如果校准过时间，把日期从BKP中读出来，同步日期
 * 
 */
static void restoreDateFromBKP(void)
{
  rtcDate.Year = HAL_RTCEx_BKUPRead(&hrtc, BKUP_REG_YEAR);
  rtcDate.Month = HAL_RTCEx_BKUPRead(&hrtc, BKUP_REG_MONTH);
  rtcDate.Date = HAL_RTCEx_BKUPRead(&hrtc, BKUP_REG_DATE);
  HAL_RTC_SetDate(&hrtc, &rtcDate, RTC_FORMAT_BIN);
}

/**
 * @brief 校准时间时，把日期存入BKP
 * 
 * @param rtcDate 日期结构体
 */
static void writeDateToBKP(RTC_DateTypeDef *rtcDate)
{
  HAL_RTCEx_BKUPWrite(&hrtc, BKUP_REG_YEAR, rtcDate->Year);
  HAL_RTCEx_BKUPWrite(&hrtc, BKUP_REG_MONTH, rtcDate->Month);
  HAL_RTCEx_BKUPWrite(&hrtc, BKUP_REG_DATE, rtcDate->Date);
}

static void setCurTimeDate(RTC_TimeTypeDef *sTime, RTC_DateTypeDef *sDate)
{
  HAL_RTC_SetTime(&hrtc, sTime, RTC_FORMAT_BIN);
  HAL_RTC_SetDate(&hrtc, sDate, RTC_FORMAT_BIN);
}

/**
 * @brief 检查BKP寄存器1（16位）是否已置标志
 * 
 * @return true 已置位
 * @return false 未置位
 */
bool RTC_CheckBkpReg1(void)
{
  if(HAL_RTCEx_BKUPRead(&hrtc, BKUP_REG_FLAG) == BKUP_FLAG_MAGIC){
    restoreDateFromBKP();
    return true;
  }
  return false;
}

/**
 * @brief 每小时触发一次
 * 
 */
void RTC_SetAlarm(void)
{
  RTC_GetCurTimeDate(&rtcTime,&rtcDate);
  RTC_AlarmTypeDef alarm = {0};
  alarm.Alarm = RTC_ALARM_A;
  alarm.AlarmTime.Hours = rtcTime.Hours + 1;
  alarm.AlarmTime.Minutes = rtcTime.Minutes;
  alarm.AlarmTime.Seconds = 0;
  HAL_RTC_SetAlarm_IT(&hrtc, &alarm, RTC_FORMAT_BIN);
}

void RTC_GetCurTimeDate(RTC_TimeTypeDef *sTime, RTC_DateTypeDef *sDate)
{
  HAL_RTC_GetTime(&hrtc,sTime,RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc,sDate,RTC_FORMAT_BIN);
}

/***********************************************/
// 先用手动校准，校准后置标志，再把年月日存入BKP
void RTC_TimeDateAdjust(void)
{
  rtcTime.Hours = 22;
  rtcTime.Minutes = 52;
  rtcTime.Seconds = 0;
  rtcDate.Year = 26;
  rtcDate.Month = 4;
  rtcDate.Date = 12;
  setCurTimeDate(&rtcTime, &rtcDate);
  HAL_RTCEx_BKUPWrite(&hrtc, BKUP_REG_FLAG, BKUP_FLAG_MAGIC);
  writeDateToBKP(&rtcDate);
}

void RTC_SetShowFlag(void)
{
  showFlag = true;
}

/**
 * @brief 主循环调用
 * 
 */
void RTC_OLED_ShowTask(void)
{
  if(showFlag){
    showFlag = false;
    RTC_GetCurTimeDate(&rtcTime, &rtcDate);

    char dateBuf[16] = {0};
    char timeBuf[16] = {0};

    snprintf(dateBuf, sizeof(dateBuf), "%04d-%02d-%02d", 2000 + rtcDate.Year, rtcDate.Month,rtcDate.Date);
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d", rtcTime.Hours, rtcTime.Minutes,rtcTime.Seconds);

    OLED_ShowString(3, 1, dateBuf);
    OLED_ShowString(4, 1, timeBuf);
  }
}

void RTC_ClearRegFlag(void)
{
  HAL_RTCEx_BKUPWrite(&hrtc, BKUP_REG_FLAG, 0);
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
  RTC_GetCurTimeDate(&rtcTime, &rtcDate);
  printf("time=%d:%d:%d\n",rtcTime.Hours,rtcTime.Minutes,rtcTime.Seconds);
  RTC_SetAlarm();
}