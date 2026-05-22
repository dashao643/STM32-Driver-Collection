#include "can_app.h"
#include "my_can.h"

#ifdef LED
#include "led.h"
#endif

#ifdef DHT11
#include "dht11.h"
#endif

#ifdef MY_RTC
#include "my_rtc.h"
#endif

bool CAN_APP_DataFrame(uint16_t stdId, const uint8_t* data, uint8_t size)
{
  if(size < 1) return false;

  switch (stdId) {
    case CAN_APP_REMOTE_ACK: {
      return true;
    }
#ifdef LED
    case CAN_APP_LED_RED: {
      if(data[0] == CAN_APP_RESET) LED_RED_OFF();
      else if(data[0] == CAN_APP_SET) LED_RED_ON();
      else LED_RED_TOGGLE();
      return true;
    }
    case CAN_APP_LED_GREEN: {
      if(data[0] == CAN_APP_RESET) LED_GREEN_OFF();
      else if(data[0] == CAN_APP_SET) LED_GREEN_ON();
      else LED_GREEN_TOGGLE();
      return true;
    }
    case CAN_APP_LED_BLUE: {
      if(data[0] == CAN_APP_RESET) LED_BLUE_OFF();
      else if(data[0] == CAN_APP_SET) LED_BLUE_ON();
      else LED_BLUE_TOGGLE();
      return true;
    }
#endif
    default:
      break;
  }
  return false;
}

HAL_StatusTypeDef CAN_APP_RemoteFrame(uint8_t stdId)
{
  switch (stdId) {
#ifdef DHT11
    case CAN_APP_DHT11: {
      uint8_t dht11Data[CAN_APP_DHT11_DATA_SIZE] = {DHT11_GetTemperature(), DHT11_GetHumidity()};
      return CAN_SendDataFrame(stdId, dht11Data, CAN_APP_DHT11_DATA_SIZE);
    }
#endif 
#ifdef MY_RTC
    case CAN_APP_RTC: {
      RTC_TimeTypeDef time;
      RTC_DateTypeDef date;
      RTC_GetCurTimeDate(&time, &date);
      // 年月日 时分秒
      uint8_t dht11Data[CAN_APP_RTC_DATA_SIZE] = {
        date.Year,
        date.Month,
        date.Date,
        time.Hours,
        time.Minutes,
        time.Seconds
      };
      return CAN_SendDataFrame(stdId, dht11Data, CAN_APP_RTC_DATA_SIZE);
    }
#endif
    default:
      break;
  }
  return HAL_ERROR;
}