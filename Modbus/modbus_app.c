#include "modbus_app.h"
#include "modbus.h"

#include <stdint.h>

#ifdef LED
#include "led.h"
#endif

#ifdef DHT11
#include "dht11.h"
#endif

#ifdef MY_RTC
#include "my_rtc.h"
#endif

#ifdef ESP8266
#include "esp8266.h"
#endif



// 检验寄存器地址
bool Modbus_App_Check_Address(uint8_t func, uint16_t addr)
{
  if((addr == 0 )|| (addr > REG_ADDR_MAX)) return false;
  if(func == MODBUS_FUNC_IAP_HANDSHAKE && addr != 0x01) return false;

  return true;
}

// 校验寄存器数量
bool Modbus_App_Check_RegCount(uint8_t func, uint16_t cnt)
{
  if((cnt == 0) || (cnt > REG_CNT_MAX)) return false;
  if(func == MODBUS_FUNC_WRITE_SINGLE_COIL && cnt != 1) return false;
  else if(func == MODBUS_FUNC_IAP_HANDSHAKE && cnt != 1) return false;

  return true;
}

// 如果是单写操作，检查操作数；多写操作，检查寄存器数和字节数
bool Modbus_App_Check_WriteValue(uint8_t func, uint16_t regCnt, uint8_t byte)
{
  if(func == MODBUS_FUNC_WRITE_SINGLE_COIL){
    if(byte > 0x04) 
      return false;
  }
  if(func == MODBUS_FUNC_WRITE_MULTI_REGS){
    if(regCnt != byte)
      return false;
  }
  return true;
}

uint16_t Modbus_App_Read_InputReg(uint16_t addr)
{
#ifdef DHT11
  if(addr == DHT11_TEMP) 
    return DHT11_GetTemperature();
  if(addr == DHT11_HUMI) 
    return DHT11_GetHumidity();
#endif
  return 0;
}

void Modbus_App_Write_Coil(uint16_t addr, uint8_t value)
{
#ifdef LED
  if(addr == LED_RED){
    if(value == MODBUS_RESET) LED_RED_OFF();
    else if(value == MODBUS_SET) LED_RED_ON();
    else LED_RED_TOGGLE();
  }
  if(addr == LED_GREEN){
    if(value == MODBUS_RESET) LED_GREEN_OFF();
    else if(value == MODBUS_SET) LED_GREEN_ON();
    else LED_GREEN_TOGGLE();
  }
  if(addr == LED_BLUE){
    if(value == MODBUS_RESET) LED_BLUE_OFF();
    else if(value == MODBUS_SET) LED_BLUE_ON();
    else LED_BLUE_TOGGLE();
  }
#endif
#ifdef ESP8266
  if(addr == ESP8266_A){
    if(value == CONNECT_SERVER) ESP8266_ConnectToServer();
  }

#endif
}

bool Modbus_App_Write_Reg(uint16_t addr, const uint8_t value[])
{
  // 设置日期和时间，地址从年开始
#ifdef MY_RTC
  if((addr != RTC_DATE_YEAR) || (value[0] != 6))
    return false;
  RTC_TimeTypeDef time = {0};
  RTC_DateTypeDef date = {0};
  date.Year = value[1];
  date.Month = value[2];
  date.Date = value[3];
  time.Hours = value[4];
  time.Minutes = value[5];
  time.Seconds = value[6];
  RTC_TimeDateAdjust(&time, &date);
#endif
  return true;
}

void Modbus_App_IAP(void)
{
  *(uint32_t*)IAP_MAGIC_ADDR = IAP_MAGIC_VAL;
  
  HAL_NVIC_SystemReset();
}

