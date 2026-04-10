#ifndef __MODBUS_APP_H__
#define __MODBUS_APP_H__

#include <stdint.h>
#include <stdbool.h>

#define LED
#define DHT11

// 寄存器地址定义
#define LED_RED		      0x0001
#define LED_GREEN 	    0x0002
#define LED_BLUE 	      0x0003
#define DHT11_TEMP	    0x0004
#define DHT11_HUMI	    0x0005
#define PWM_LED	        0x0006

#define REG_ADDR_MAX	  0x0006  // 最大寄存器地址
#define REG_CNT_MAX     6       // 寄存器地址数量

// 支持的操作数
#define MODBUS_RESET 	  0x00    // 关闭
#define MODBUS_SET 	    0x01    // 开启
#define MODBUS_TOGGLE 	0x02    // 翻转

bool Modbus_App_Check_Address(uint16_t addr);
bool Modbus_App_Check_RegCount(uint16_t cnt);
bool Modbus_App_Check_WriteValue(uint8_t value);

void Modbus_App_Write_Coil(uint16_t addr, uint8_t value);
uint16_t Modbus_App_Read_InputReg(uint16_t addr);

#endif