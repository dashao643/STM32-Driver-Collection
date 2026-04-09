#include "modbus.h"
#include "usart.h"
#include "general.h"
#include "modbus_app.h"
#include "stm32f1xx.h"
#include "stm32f1xx_hal.h"

#include <string.h>

static Modbus_Uart_HandleTypeDef modbus;
static Modbus_Frame_Record modbusRecord;

static uint8_t modbus_tx_buff[MODBUS_TX_BUFF_MAXLENTH];
static uint8_t modbus_tx_index = 0;

static void clearRxBuffer(void);
static void clearRecord(void);
static bool lengthCheck(void);
static bool addressCheck(void);
static bool funcCheck(void);
static bool regArrCheck(void);
static bool regCntCheck(void);
static bool opDataCheck(void);
static bool crcCheck(void);
static void errorFrameReply(uint8_t errorCode);
static void frameCheck(void);
static void frameProcess(void);
static void frameReply(void);

static void clearRxBuffer(void)
{
  modbus.index = 0;
  modbus.overflow = false;
  modbus.frameEnd = false;
}

static void clearRecord(void)
{
  modbusRecord.funcCode = 0;
  modbusRecord.regArr = 0;
  modbusRecord.regCnt = 0;
  modbusRecord.isRead = false;
  modbusRecord.dataCode = 0;
}

static bool lengthCheck(void)
{
  if((modbus.index < MODBUS_RX_BUFF_MINLENTH) || (modbus.overflow))
    return false;
  return true;
}

static bool addressCheck(void)
{
  if(modbus.buffer[0] != MODBUS_SLAVE_ADDR)
    return false;
  return true;
}

/**
 * @brief 功能码校验，注释的为暂不支持
 * 
 * @return true 校验成功
 * @return false 校验失败
 */
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
    modbusRecord.isRead = true;
    break;
  }
  case MODBUS_FUNC_WRITE_SINGLE_COIL:{
    modbusRecord.isRead = false;
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
    return false;
  }

  modbusRecord.funcCode = modbus.buffer[1];
  return true;
}

static bool regArrCheck(void)
{
  U16Union regArr = {0};
  // 先赋值低字节，再赋值高字节
  regArr.bytes[0] = modbus.buffer[3];
  regArr.bytes[1] = modbus.buffer[2];
  
  if(!Modbus_App_Check_Address(regArr.word))
    return false;

  modbusRecord.regArr = regArr.word;
  return true;
}

static bool regCntCheck(void)
{
  U16Union regCnt = {0};
  // 先赋值低字节，再赋值高字节
  regCnt.bytes[0] = modbus.buffer[5];
  regCnt.bytes[1] = modbus.buffer[4];
  
  if((modbusRecord.funcCode == MODBUS_FUNC_WRITE_SINGLE_COIL) && (regCnt.word != 1))
    return false;
  if(!Modbus_App_Check_RegCount(regCnt.word))
    return false;

  modbusRecord.regCnt = regCnt.word;
  return true;
}

static bool opDataCheck(void)
{
  if(!Modbus_App_Check_WriteValue(modbus.buffer[6]))
    return false;
  modbusRecord.dataCode = modbus.buffer[6];
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

static void errorFrameReply(uint8_t errorCode)
{
  U16Union crcCal = {0};
  uint8_t reply[5] = {MODBUS_SLAVE_ADDR,modbus.buffer[1] | 0x80,errorCode,0,0};
  crcCal.word = CRC16_Modbus(reply, 3);
  reply[3] = crcCal.bytes[0];
  reply[4] = crcCal.bytes[1];
  HAL_UART_Transmit(MODBUS_UARTX, reply, sizeof(reply), MODBUS_UARTX_TIMEOUT);
}

/**
 * @brief 状态机实现帧验证
 * 
 */
static void frameCheck(void)
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
      if(!funcCheck()){
        errorFrameReply(MODBUS_FUNC_ERROR);
        return;
      }
      state = MODBUS_STATE_REG_ADDR;
      break;
    }
    case MODBUS_STATE_REG_ADDR:{
      if(!regArrCheck()){
        errorFrameReply(MODBUS_REGS_ARR_ERROR);
        return;
      }
      state = MODBUS_STATE_REG_CNT;
      break;
    }
    case MODBUS_STATE_REG_CNT:{
      if(!regCntCheck()){
        errorFrameReply(MODBUS_REGS_CNT_ERROR);
        return;
      }
      // 读操作
      if(modbusRecord.isRead)
        state = MODBUS_STATE_CRC;
      // 写操作
      else
        state = MODBUS_STATE_DATA;
      break;
    }
    case MODBUS_STATE_DATA:{
      if(!opDataCheck()){
        errorFrameReply(MODBUS_OP_DATA_ERROR);
        return;
      }
      state = MODBUS_STATE_CRC;
      break;
    }
    case MODBUS_STATE_CRC:{
      if(!crcCheck())
        return;
      state = MODBUS_STATE_PROCESS;
      break;
    }
    case MODBUS_STATE_PROCESS:{
      frameProcess();
      state = MODBUS_STATE_REPLY;
      break;
    }
    case MODBUS_STATE_REPLY:{
      frameReply();
      state = MODBUS_STATE_REPLY;
      break;
    }
    case MODBUS_STATE_RESET:{
      clearRxBuffer();
      clearRecord();
      modbus_tx_index = 0;
      state = MODBUS_STATE_IDLE;
      return;
    }
    default:
      return;
    }
  }
}

/**
 * @brief 根据记录的帧，实现帧处理，帧回复
 * 
 */
static void frameProcess(void)
{
  modbus_tx_buff[modbus_tx_index++] = MODBUS_SLAVE_ADDR;
  modbus_tx_buff[modbus_tx_index++] = modbusRecord.funcCode;
  // 读操作需要返回数据
  if(modbusRecord.isRead){
    // 纯数据长度，每个寄存器 2 字节
    uint8_t byteCnt = modbusRecord.regCnt * 2;
    modbus_tx_buff[modbus_tx_index++] = byteCnt;
  }
  // 写操作直接拷贝原始帧
  U16Union regData = {0};
  switch (modbusRecord.funcCode) {
  /*********************** 读 *************************/
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
    for(uint8_t i = 0;i < modbusRecord.regCnt; i++){
      regData.word = Modbus_App_Read_InputReg(modbusRecord.regArr + i);
      // 数据先传高字节，再传低字节
      modbus_tx_buff[modbus_tx_index++] = regData.bytes[1];
      modbus_tx_buff[modbus_tx_index++] = regData.bytes[0];
    }
    break;
  }
  /*********************** 写 *************************/
  case MODBUS_FUNC_WRITE_SINGLE_COIL:{
    Modbus_App_Write_Coil(modbusRecord.regArr,modbusRecord.dataCode);
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
  }
  return;
}

static void frameReply(void)
{
  if(modbusRecord.isRead){
    U16Union CRC16 = {0};
    // 计算CRC，低字节在前，高字节在后
    CRC16.word = CRC16_Modbus(modbus_tx_buff,modbus_tx_index);
    modbus_tx_buff[modbus_tx_index++] = CRC16.bytes[0];
    modbus_tx_buff[modbus_tx_index++] = CRC16.bytes[1];
  }
  else{
    memcpy(modbus_tx_buff, modbus.buffer, modbus.index);
  }
  HAL_UART_Transmit(MODBUS_UARTX,modbus_tx_buff,modbus_tx_index,MODBUS_UARTX_TIMEOUT);
}

/**
 * @brief 初始化Modbus结构体，先开启中断
 * 
 */
void Modbus_UART_Init(void)
{
  clearRxBuffer();
  clearRecord();
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
    frameCheck();

    HAL_UART_Receive_IT(MODBUS_UARTX, modbus.buffer, 1);
  }
}

