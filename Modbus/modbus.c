#include "modbus.h"
#include "usart.h"
#include "general.h"
#include "stm32f1xx.h"
#include "stm32f1xx_hal.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "gpio.h"

static Modbus_Uart_HandleTypeDef modbus;

static void clearRxBuffer(void);
static bool lengthCheck(void);
static bool addressCheck(void);
static bool funcCheck(void);
static bool regArrCheck(void);
static bool regCntCheck(void);
static bool dataCheck(void);
static bool crcCheck(void);
static void frameProcess(void);
static void errorFrameReply(uint8_t errorCode);

static void clearRxBuffer(void)
{
  modbus.index = 0;
  modbus.overflow = false;
  modbus.frameEnd = false;
}

static bool lengthCheck(void)
{
  if(modbus.index < MODBUS_RX_BUFF_MINLENTH)
    return false;
  return true;
}

static bool addressCheck(void)
{
  if(modbus.buffer[0] != MODBUS_SLAVE_ADDR)
    return false;
  return true;
}

static bool funcCheck(void)
{
  switch (modbus.buffer[1]) {
  // case MODBUS_FUNC_READ_COILS:{
  //   break;
  // }
  // case MODBUS_FUNC_READ_DISCRETE_INPUT:{
  //   break;
  // }
  // case MODBUS_FUNC_READ_HOLD_REGS:{
  //   break;
  // }
  case MODBUS_FUNC_READ_INPUT_REGS:{
    break;
  }
  case MODBUS_FUNC_WRITE_SINGLE_COIL:{
    break;
  }
  // case MODBUS_FUNC_WRITE_SINGLE_REG:{
  //   break;
  // }
  // case MODBUS_FUNC_WRITE_MULTI_COILS:{
  //   break;
  // }
  // case MODBUS_FUNC_WRITE_MULTI_REGS:{
  //   break;
  // }
  default:
    errorFrameReply(MODBUS_FUNC_ERROR);
    return false;
  }

  return true;
}

static bool regArrCheck(void)
{
  U16Union regArr = {0};
  // 先赋值低字节，再赋值高字节
  regArr.bytes[0] = modbus.buffer[3];
  regArr.bytes[1] = modbus.buffer[2];
  if((regArr.word == 0) || (regArr.word > REG_ARR_MAX)){
    errorFrameReply(MODBUS_REGS_ARR_ERROR);
    return false;
  }
  return true;
}

static bool regCntCheck(void)
{
  U16Union regCnt = {0};
  // 先赋值低字节，再赋值高字节
  regCnt.bytes[0] = modbus.buffer[5];
  regCnt.bytes[1] = modbus.buffer[4];
  if((regCnt.word == 0) || (regCnt.word > REG_ARR_CNT)){
    errorFrameReply(MODBUS_REGS_CNT_ERROR);
    return false;
  }
  return true;
}

static bool dataCheck(void)
{
  switch (modbus.buffer[6]) {
  case MODBUS_RESET:{
    break;
  }
  case MODBUS_SET:{
    break;
  }
  case MODBUS_TOGGLE:{
    break;
  }
  default:
    errorFrameReply(MODBUS_DATA_ERROR);
    return false;
  }
  return true;
} 

static bool crcCheck(void)
{
  U16Union crcRes = {0};
  // 最后两字节是收到的CRC，不参与计算
  crcRes.word = CRC16_Modbus(modbus.buffer,modbus.index - 2);
  // crc16低位 -- 数据传输先传低字节
  if( crcRes.bytes[0] == modbus.buffer[modbus.index-2] &&
    crcRes.bytes[1] == modbus.buffer[modbus.index-1] )
    return true;
  return false;
}

/**
 * @brief 状态机实现帧处理
 * 
 */
static void frameProcess(void)
{
  Modbus_StateTypeDef state = MODBUS_STATE_IDLE;
  while(1){
    switch (state) {
    case MODBUS_STATE_IDLE:{
      if(!lengthCheck())
        return;
      state = MODBUS_STATE_ADDR;
      break;
    }
    case MODBUS_STATE_ADDR:{
      if(!addressCheck())
        return;
      state = MODBUS_STATE_FUNC;
      break;
    }
    case MODBUS_STATE_FUNC:{
      if(!funcCheck())
        return;
      state = MODBUS_STATE_REG_ADDR;
      break;
    }
    case MODBUS_STATE_REG_ADDR:{
      if(!regArrCheck())
        return;
      state = MODBUS_STATE_REG_CNT;
      break;
    }
    case MODBUS_STATE_REG_CNT:{
      if(!regCntCheck())
        return;
      // 读操作
      if((modbus.buffer[1] >= MODBUS_FUNC_READ_COILS) && (modbus.buffer[1] <= MODBUS_FUNC_READ_INPUT_REGS))
        state = MODBUS_STATE_CRC;
      // 写操作
      else
        state = MODBUS_STATE_DATA;
      break;
    }
    case MODBUS_STATE_DATA:{
      if(!dataCheck())
        return;
      state = MODBUS_STATE_CRC;
      break;
    }
    case MODBUS_STATE_CRC:{
      if(!crcCheck())
        return;
      LED_GREEN_TOGGLE();
      return;
      break;
    }
    case MODBUS_STATE_PROCESS:{
      Modbus_App_FrameProcess();
      break;
    }
    case MODBUS_STATE_REPLY:{
      break;
    }
    case MODBUS_STATE_RESET:{
      break;
    }
    default:
      break;
    }
  }
}

static void errorFrameReply(uint8_t errorCode)
{
  uint8_t reply[3] = {MODBUS_SLAVE_ADDR,modbus.buffer[1] | 0x80,errorCode};
  HAL_UART_Transmit(MODBUS_UARTX, reply, sizeof(reply), MODBUS_UARTX_TIMEOUT);
}

/**
 * @brief 初始化Modbus结构体，先开启中断
 * 
 */
void Modbus_UART_Init(void)
{
  clearRxBuffer();
  __HAL_UART_ENABLE_IT(MODBUS_UARTX,UART_IT_IDLE);
  HAL_UART_Receive_IT(MODBUS_UARTX, modbus.buffer, 1);
}

void Modbus_UART_SetFrameEndFlag(void)
{
  modbus.frameEnd = true;
}

void Modbus_UART_SingleByte(void)
{
  if(modbus.overflow == false){
    modbus.index++;
    if (modbus.index <= MODBUS_RX_BUFF_MAXLENTH)
      // 回调中重新开启中断接收，无窗口期
      HAL_UART_Receive_IT(MODBUS_UARTX, modbus.buffer + modbus.index, 1);
    else
      modbus.overflow = true;
  }
}

/**
 * @brief 置IDLE标志位后，表明数据传输结束，调用此函数
 * 
 */
void Modbus_UART_Task(void)
{
  if(modbus.frameEnd){
    // 先中止正在进行的接收（强制把 HAL 状态恢复为 READY，使中断接收开启一定生效）
    HAL_UART_AbortReceive_IT(MODBUS_UARTX);
    // 没有溢出，才解析数据
    if (modbus.overflow == false){
      // printf("%d\n",modbus.index);
      frameProcess();
      // if(modbus.buffer[0] == 0x01)
      //   LED_RED_TOGGLE();
      // else if(modbus.buffer[0] == 0x02)
      //   LED_GREEN_TOGGLE();
    }
    else
      printf("frame too long\n");

    clearRxBuffer();
    HAL_UART_Receive_IT(MODBUS_UARTX, modbus.buffer, 1);
  }
}

void Modbus_App_FrameProcess(void)
{

}
