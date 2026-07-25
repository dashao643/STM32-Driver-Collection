#include "stm32f4xx_hal.h"
#include "my_rtc.h"

// // 把日期写入BKP寄存器，掉电后VBAT保持数据
// static void saveDateToBKP(const RTC_DateTypeDef *sDate)
// {
//   HAL_RTCEx_BKUPWrite(&hrtc, BKUP_REG_YEAR,  sDate->Year);
//   HAL_RTCEx_BKUPWrite(&hrtc, BKUP_REG_MONTH, sDate->Month);
//   HAL_RTCEx_BKUPWrite(&hrtc, BKUP_REG_DATE,  sDate->Date);
// }

// // 从BKP寄存器恢复日期
// static void loadDateFromBKP(RTC_DateTypeDef *sDate)
// {
//   sDate->Year  = HAL_RTCEx_BKUPRead(&hrtc, BKUP_REG_YEAR);
//   sDate->Month = HAL_RTCEx_BKUPRead(&hrtc, BKUP_REG_MONTH);
//   sDate->Date  = HAL_RTCEx_BKUPRead(&hrtc, BKUP_REG_DATE);
// }

/*-----------------------------------------------------------------*/

// 检查RTC是否被校准过(BKP中是否有标志),在 MX_RTC_Init 中调用
bool RTC_IsConfigured(void)
{
  return HAL_RTCEx_BKUPRead(&hrtc, BKUP_REG_FLAG) == BKUP_FLAG_MAGIC;
  // RTC_DateTypeDef sDate;
  // // loadDateFromBKP(&sDate);
  // // 恢复日期影子寄存器，这样后续GetTime检测到天溢出时能正确推进日期
  // HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
  // return true;
}

// 读取当前日期时间:必须先GetTime再GetDate，GetTime内部会检测天溢出并更新日期影子
void RTC_GetDateTime(RTC_DateTypeDef *date, RTC_TimeTypeDef *time)
{
  HAL_RTC_GetTime(&hrtc, time, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, date, RTC_FORMAT_BIN);
}

// 设置日期时间:必须先SetDate(写入日期影子)再SetTime(写入CNT计数器),否则日期不生效
void RTC_SetDateTime(RTC_DateTypeDef *date, RTC_TimeTypeDef *time)
{
  HAL_RTC_SetDate(&hrtc, date, RTC_FORMAT_BIN);
  HAL_RTC_SetTime(&hrtc, time, RTC_FORMAT_BIN);

  HAL_RTCEx_BKUPWrite(&hrtc, BKUP_REG_FLAG, BKUP_FLAG_MAGIC);
}

// 清除BKP标志(恢复出厂设置)
void RTC_ResetConfig(void)
{
  HAL_RTCEx_BKUPWrite(&hrtc, BKUP_REG_FLAG, 0);
}

// 在 RTC_IRQHandler 中调用 HAL_RTC_AlarmIRQHandler
// 能够唤醒 F1 待机模式 不能唤醒 停止模式
// 传入 NULL 为默认 5s 后触发闹钟中断
void RTC_SetAlarm_IT(const RTC_TimeTypeDef *time)
{
  // The HAL_RTC_SetTime() must be called before enabling the Alarm feature.
  RTC_DateTypeDef curDate;
  RTC_TimeTypeDef curTime;
  RTC_GetDateTime(&curDate, &curTime);
  RTC_SetDateTime(&curDate, &curTime);

  RTC_AlarmTypeDef alarm;
  alarm.Alarm = RTC_ALARM_A;
  if(time) {
    alarm.AlarmTime.Hours = time->Hours;
    alarm.AlarmTime.Minutes = time->Minutes;
    alarm.AlarmTime.Seconds = time->Seconds;
  } else {
    alarm.AlarmTime.Hours = curTime.Hours;
    alarm.AlarmTime.Minutes = curTime.Minutes;
    alarm.AlarmTime.Seconds = curTime.Seconds + 5; // 不考虑溢出
  }
  HAL_RTC_SetAlarm_IT(&hrtc, &alarm, RTC_FORMAT_BIN);
}