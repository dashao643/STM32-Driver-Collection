#ifndef __MODBUS_APP_H__
#define __MODBUS_APP_H__

#include <stdint.h>
#include <stdbool.h>

#define LED
#define DHT11
#define MY_RTC

// 线圈地址定义
#define LED_RED		      0x0001
#define LED_GREEN 	    0x0002
#define LED_BLUE 	      0x0003
#define PWM_LED	        0x0004

// 输入寄存器地址定义
#define DHT11_TEMP	    0x0001
#define DHT11_HUMI	    0x0002

// 保持寄存器地址定义
#define RTC_DATE_YREA      0x0001
#define RTC_DATE_MONTH     0x0002
#define RTC_DATE_DAY       0x0003
#define RTC_TIME_HOUR      0x0004
#define RTC_TIME_MINUTE    0x0005
#define RTC_TIME_SECOND    0x0006

// 支持的操作数
#define MODBUS_RESET 	  0x00    // 关闭
#define MODBUS_SET 	    0x01    // 开启
#define MODBUS_TOGGLE 	0x02    // 翻转

#define REG_ADDR_MAX	  0x0006  // 最大寄存器地址
#define REG_CNT_MAX     6       // 寄存器地址数量

bool Modbus_App_Check_Address(uint16_t addr);
bool Modbus_App_Check_RegCount(uint16_t cnt);
bool Modbus_App_Check_WriteValue(uint8_t func, uint16_t regCnt, uint8_t byte);

uint16_t Modbus_App_Read_InputReg(uint16_t addr);
void Modbus_App_Write_Coil(uint16_t addr, uint8_t value);
bool Modbus_App_Write_Reg(uint16_t addr, uint8_t value[]);

#endif