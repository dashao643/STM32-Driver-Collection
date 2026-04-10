#include "modbus_app.h"

#include "gpio.h"
#include "dht11.h"

bool Modbus_App_Check_Address(uint16_t addr)
{
  if((addr == 0 )|| (addr > REG_ADDR_MAX)) 
    return false;
  return true;
}

bool Modbus_App_Check_RegCount(uint16_t cnt)
{
  if((cnt == 0) || (cnt > REG_CNT_MAX)) 
    return false;
  return true;
}

bool Modbus_App_Check_WriteValue(uint8_t value)
{
  if(value > 2) 
    return false;
  return true;
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
