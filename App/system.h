#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include <stdint.h>
#include <stdbool.h>

void System_Init(void);

void DHT11_Task(void);
void RTC_Task(void);
void HC_SR04_Task(void);

// #define SYSTEM_EVENT_MY_RTC				  (uint32)(0x01<<0)												       	// 校时
// #define SYSTEM_EVENT_INFRARED			  (uint32)(0x01<<1)                              	// 红外
// #define SYSTEM_EVENT_DEBUG   			  (uint32)(0x01<<3)                              	// 串口
// #define SYSTEM_EVENT_ATTACKCHANGE   (uint32)(0x01<<5)                             	// 攻击状态改变
// #define SYSTEM_EVENT_VALVEACTION    (uint32)(0x01<<6)                              	// 30天洗阀门
// #define SYSTEM_EVENT_RESET          (uint32)(0x01<<8)                             	// 复位
// #define SYSTEM_EVENT_FAULT				  (uint32)(0x01<<9)                             	// 故障
// #define SYSTEM_EVENT_VALVEOPEN		  (uint32)(0x01<<10)                             	// 开阀
// #define SYSTEM_EVENT_VALVECLOSE		  (uint32)(0x01<<11)                             	// 关阀


#endif
