#ifndef __MODBUS_H__
#define __MODBUS_H__

#include "gpio.h"
#include <stdint.h>
#include <stdbool.h>

#define MODBUS_UARTX                    &huart1
#define MODBUS_UARTX_TIMEOUT            500
#define MODBUS_RX_BUFF_MAXLENTH         10      // 最大帧长度
#define MODBUS_RX_BUFF_MINLENTH         8       // 最小帧长度

#define MODBUS_SLAVE_ADDR               0x01    // 从机地址

#define MODBUS_FUNC_READ_COILS          0x01    // 读线圈（输出IO）
#define MODBUS_FUNC_READ_DISCRETE_INPUT 0x02    // 读离散输入（输入IO）
#define MODBUS_FUNC_READ_HOLD_REGS      0x03    // 读保持寄存器（可读可写变量）
#define MODBUS_FUNC_READ_INPUT_REGS     0x04    // 读输入寄存器（只读变量）       // 支持
#define MODBUS_FUNC_WRITE_SINGLE_COIL   0x05    // 写单个线圈                    // 支持
#define MODBUS_FUNC_WRITE_SINGLE_REG    0x06    // 写单个保持寄存器
#define MODBUS_FUNC_WRITE_MULTI_COILS   0x0F    // 写多个线圈
#define MODBUS_FUNC_WRITE_MULTI_REGS    0x10    // 写多个保持寄存器     

// 错误码
#define MODBUS_FUNC_ERROR               0x01    // 非法功能码
#define MODBUS_REGS_ARR_ERROR           0x02    // 非法寄存器地址
#define MODBUS_REGS_CNT_ERROR           0x03    // 非法寄存器数量
#define MODBUS_DATA_ERROR               0x04    // 非法操作数

// 寄存器地址
#define LED_RED		      0x0001
#define LED_GREEN 	    0x0002
#define LED_BLUE 	      0x0003
#define DHT11_TEMP	    0x0004
#define DHT11_HUMI	    0x0005
#define PWM_LED	        0x0006

#define REG_ARR_MAX	    0x0006  // 最大寄存器地址
#define REG_ARR_CNT     6       // 寄存器地址数量

// 支持操作数
#define MODBUS_RESET 	  0x00    // 关闭
#define MODBUS_SET 	    0x01    // 开启
#define MODBUS_TOGGLE 	0x02    // 翻转

typedef struct{
  uint8_t buffer[MODBUS_RX_BUFF_MAXLENTH];
  uint16_t index;
  bool overflow;
  bool frameEnd;
}Modbus_Uart_HandleTypeDef;

typedef enum {
  MODBUS_STATE_IDLE = 0,        // 空闲，等待一帧开始
  MODBUS_STATE_ADDR,            // 接收并校验从机地址
  MODBUS_STATE_FUNC,            // 接收功能码
  MODBUS_STATE_REG_ADDR,
  MODBUS_STATE_REG_CNT,
  MODBUS_STATE_DATA,            // 接收数据段（写操作用到）（一个字节）
  MODBUS_STATE_CRC,             // 校验CRC
  MODBUS_STATE_PROCESS,         // 执行功能（读取数据/控制IO）
  MODBUS_STATE_REPLY,           // 构造回复帧
  MODBUS_STATE_RESET            // 重置状态机，准备下一帧
} Modbus_StateTypeDef;

void Modbus_UART_Init(void);
void Modbus_UART_SetFrameEndFlag(void);
void Modbus_UART_SingleByte(void);
void Modbus_UART_Task(void);

void Modbus_App_FrameProcess(void);

#endif