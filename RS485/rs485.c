#include "rs485.h"
#include "stm32f1xx_hal.h"

void RS485_Transmit(RS485_t *rs485, uint16_t size, const TransmitMode_e mode, const uint32_t timeout)
{
  RS485_TRANSMIT_MODE();
  UART_Transmit(&rs485->uart, size, mode, timeout);

  // 挂起一个标志位，task里面判断
  rs485->transmitFlag = true;
  rs485->timer = HAL_GetTick();
}

/**
 * @brief 上层task函数中调用，用于恢复接收状态
 * 
 * @param rs485 
 */
void RS485_Task(RS485_t *rs485)
{
  if(rs485->transmitFlag){
    if((HAL_GetTick() - rs485->timer) > RESTORE_TIME_MS){
      RS485_RECEIVE_MODE();
      rs485->transmitFlag = false;
    }
  }
}
