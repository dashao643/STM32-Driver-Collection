#include "modbus_usart.h"
#include "usart.h"
#include "stm32f1xx.h"
#include "stm32f1xx_hal.h"

#include <stdio.h>
#include <string.h>

#include "gpio.h"
#include "oled.h"

static modbus_usart_rx_t modbus_usart;

static void Modbus_ClearRxBuffer(void);

/**
 * @brief printf函数重定向到串口1
 * 
 * @param ch 
 * @return int 
 */
int __io_putchar(int ch)
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, USART1_TIMEOUT);
  
  return ch;
}

static void Modbus_ClearRxBuffer(void)
{
  modbus_usart.tempByte = 0x00;
  memset(modbus_usart.buffer, 0, sizeof(modbus_usart.buffer));
  modbus_usart.index = 0;
}

/**
 * @brief 初始化Modbus结构体，先开启中断
 * 
 */
void Modbus_UART_Init(void)
{
  Modbus_ClearRxBuffer();
  __HAL_UART_ENABLE_IT(&huart1,UART_IT_IDLE);
  HAL_UART_Receive_IT(&huart1, &modbus_usart.tempByte, 1);
}

void Modbus_UART_SingleByte(void)
{
  // LED_GREEN_TOGGLE();
  // OLED_ShowDecNumber(2,1,modbus_usart.index,3);
  if (modbus_usart.index < RX_BUFF_MAXLENTH) {
    modbus_usart.buffer[modbus_usart.index++] = modbus_usart.tempByte;
  }
  // 回调中重新开启中断接收，无窗口期
  HAL_UART_Receive_IT(&huart1, &modbus_usart.tempByte, 1);
}

/**
 * @brief 置IDLE标志位后，表明数据传输结束，调用此函数处理数据
 * 
 */
void Modbus_UART_FrameEnd(void)
{
  static uint16_t frame_count = 0;
  frame_count++;
  OLED_ShowDecNumber(2, 1, frame_count, 3);  // 显示收到多少帧
  OLED_ShowDecNumber(2, 5, modbus_usart.index, 3);  // 显示帧长度

  if (modbus_usart.index > 0) {
    HAL_UART_Transmit(&huart1, modbus_usart.buffer, modbus_usart.index, 100);
  }
  Modbus_ClearRxBuffer();
}

