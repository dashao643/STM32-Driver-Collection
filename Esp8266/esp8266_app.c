#include "stm32f1xx_hal.h"
#include "esp8266_app.h"
#include "at24c64_app.h"
#include "led.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "modbus.h"

/********************* 命令指令 *******************/
void Fun_LedRedToggle(void) { LED_RED_TOGGLE(); }
void Fun_LedGreenToggle(void) { LED_GREEN_TOGGLE(); }
void Fun_LedBlueToggle(void) { LED_BLUE_TOGGLE(); }
void Fun_LedAllToggle(void) { LED_RED_TOGGLE(); LED_GREEN_TOGGLE(); LED_BLUE_TOGGLE(); }

CmdTable_t cmdTable[] = {
  { "LED_RED_TOGGLE" ,   Fun_LedRedToggle},
  { "LED_GREEN_TOGGLE" , Fun_LedGreenToggle},
  { "LED_BLUE_TOGGLE" ,  Fun_LedBlueToggle},
  { "LED_ALL_TOGGLE" ,  Fun_LedAllToggle}
};
#define CMD_CNT         (sizeof(cmdTable)/sizeof(CmdTable_t))

/********************* 数据指令 *******************/
bool Fun_WiFiConfig(const char* str) 
{ 
	char ssid[33] = { 0 };
	char pwd[33] = { 0 };

	// 按照格式提取字符串
	if (sscanf(str, "WiFi_CONFIG:\"%32[^\"]\",\"%32[^\"]\"", ssid, pwd) != 2) {
		return false;
	}

	// printf("ssid=%s\n", ssid);
	// printf("pwd=%s\n", pwd);
  // 写入EE存储，结束符必须存储
  // uint8_t test = 0xFF;

  // Modbus_Transmit(&test, 1);
  // Modbus_Transmit((uint8_t*)ssid, strlen(ssid) + 1);
  // Modbus_Transmit(&test, 1);
  // Modbus_Transmit((uint8_t*)pwd, strlen(pwd) + 1);
  // Modbus_Transmit(&test, 1);    
  AT24C64_App_WriteWiFiSSID(ssid, strlen(ssid) + 1);
  // 等待写入间隔
  HAL_Delay(5);
  AT24C64_App_WriteWiFiPassword(pwd, strlen(pwd) + 1);

  return true;
}

DataTable_t dataTable[] = {
	{ "WiFi_CONFIG:", Fun_WiFiConfig}
};
#define DATA_CNT         (sizeof(dataTable)/sizeof(DataTable_t))

ESP8266_APP_RES_e ESP8266_APP_LocalCmd(const char *cmdStr, uint16_t size)
{
  Modbus_Transmit((uint8_t*)cmdStr, size);
  
  // 查询控制指令
  for(uint8_t i = 0; i < CMD_CNT; i++){
    if(strcmp(cmdStr, cmdTable[i].cmdStr) == 0){
      cmdTable[i].pFunc();
      return CMD_OK;
    }
  }

  // 配网指令
  for(uint8_t i = 0; i < DATA_CNT; i++){
    if(strstr(cmdStr, dataTable[i].dataStr)){
      if(dataTable[i].pFunc(cmdStr))
        return WiFi_CONFIG_OK;
      return DATA_ERROR;
    }
  }

  return CMD_ERROR;
}