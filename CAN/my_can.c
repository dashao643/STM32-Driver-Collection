#include "my_can.h"
#include "stm32f1xx_hal.h"
#include "can_app.h"

#include <stdint.h>
#include <stdio.h>
#include "gpio.h"

// APB1时钟36Mhz 
// PSC: 6
// BS1(8) + BS2(3) : 11(8 - 16)
// SJW: 1
// bsp = 36 000 000 / 6 / (11 + 1)

// 屏蔽位值为1，表示要匹配;
// 为0表示不需要匹配

// 映像:11位STID + 18位EXID + IDE + RTR + 0 = 32

static CAN_RxQueue_t can = {0};
static uint32_t txTimer = 0;

static void setFilter(void)
{
  CAN_FilterTypeDef filter;
  
  // 过滤器配置
  filter.FilterBank = 0;
  filter.FilterMode = CAN_FILTERMODE_IDMASK;
  filter.FilterScale = CAN_FILTERSCALE_32BIT;

  // id范围：0 - 127，低7位放行，高4位固定
  filter.FilterIdHigh   = 0x0000;
  filter.FilterIdLow    = 0x0000;
  filter.FilterMaskIdHigh = 0xF000;
  filter.FilterMaskIdLow  = 0x0000;

  filter.FilterActivation = CAN_FILTER_ENABLE;
  filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  filter.SlaveStartFilterBank = 0;
  
  HAL_CAN_ConfigFilter(CAN_HANDLE, &filter);
}

// 串口打印
static void dataProcess(void)
{
  while(can.rxIn != can.rxOut){
    CAN_Rx_t *frame = &can.rxQueue[can.rxOut];
    printf("-------------------------------------\r\n");
    printf("IDE:%d\r\n",    (int)frame->rxHeader.IDE);
    printf("StdId:%d\r\n",  (int)frame->rxHeader.StdId);
    printf("DLC:%d\r\n",    (int)frame->rxHeader.DLC);
    printf("RTR:%d\r\n",    (int)frame->rxHeader.RTR);
    // 接收到了数据帧,显示数据
    if(can.rxQueue[can.rxOut].rxHeader.RTR == CAN_RTR_DATA){
      for(uint8_t i = 0; i < frame->rxHeader.DLC; i++){
        printf("data[%d]=%d\n",i,frame->data[i]);
      }
      printf("\r\n");
    }
    // 接收到了遥控帧,回传数据,待实现
    else {
      
    }
    can.rxOut = (can.rxOut + 1) % CAN_RX_QUEUE_NUM;
  }
}

void CAN_Init(void)
{
  setFilter();
  HAL_CAN_Start(CAN_HANDLE);
  HAL_CAN_ActivateNotification(CAN_HANDLE, CAN_IT_RX_FIFO0_MSG_PENDING);
}

void CAN_Task(void)
{
  // 两个指针相等表示队列为空
  if(can.rxIn == can.rxOut){
    return;
  }
  // canRx.rxFlag = false;
  LED_GREEN_TOGGLE();
  // 解析数据，此处可调用应用层函数
  dataProcess();
}

/**
 * @brief CAN发送函数
 * 
 * @param txHeader 数据头结构体
 * @param data 数组指针
 * @param size 数据大小
 * @return HAL_StatusTypeDef 返回状态
 */
HAL_StatusTypeDef CAN_Transmit(const CAN_TxHeader_t *txHeader, const uint8_t *data, uint8_t size)
{
  if(txHeader->stdId > CAN_STD_ID_MAX) return HAL_ERROR;
  if((txHeader->rtr != CAN_RTR_DATA) && (txHeader->rtr != CAN_RTR_REMOTE)) return HAL_ERROR;
  if((txHeader->rtr == CAN_RTR_DATA) && (size == 0)) return HAL_ERROR;
  if((txHeader->rtr == CAN_RTR_REMOTE) && (size != 0)) return HAL_ERROR;
  if(size > CAN_DATA_SIZE_MAX) return HAL_ERROR;

  CAN_TxHeaderTypeDef TxHeader = {0};

  TxHeader.IDE = CAN_ID_STD;                      // 指定ID类型
  TxHeader.StdId = txHeader->stdId;               // 配置ID
  TxHeader.RTR = txHeader->rtr;                   // 指定帧类型
  TxHeader.DLC = size;                            // 数据长度 0 - 8
  TxHeader.TransmitGlobalTime = DISABLE;          // 时间戳设置

  // 等待可用的发送邮箱
  txTimer = HAL_GetTick();
  while(HAL_CAN_GetTxMailboxesFreeLevel(CAN_HANDLE) == 0){
    if((HAL_GetTick() - txTimer) > CAN_TX_TIMEOUT_MS)
      return HAL_TIMEOUT;
  }
  // 返回实际使用的邮箱编号
  uint32_t txMailbox;
  uint8_t state;
  state = HAL_CAN_AddTxMessage(CAN_HANDLE, &TxHeader, data, &txMailbox);
  // 等待发送完成
  if(state == HAL_OK){
    txTimer = HAL_GetTick();
    while(HAL_CAN_IsTxMessagePending(CAN_HANDLE, txMailbox)){
      if((HAL_GetTick() - txTimer) > CAN_TX_TIMEOUT_MS)
        return HAL_TIMEOUT;
    }
  }
  return state;
}

// 中断回调中读走数据,存入队列
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  if(hcan->Instance == CAN_INSTANCE){
    CAN_Rx_t tempFrame;
    HAL_CAN_GetRxMessage(hcan,CAN_RX_FIFO0,&tempFrame.rxHeader,tempFrame.data);
    uint8_t nextIn = (can.rxIn + 1) % CAN_RX_QUEUE_NUM;
    // 舍弃一个位置，用于表示队列已满
    if (nextIn == can.rxOut){
      LED_BLUE_TOGGLE();
      return;
    }
    can.rxQueue[can.rxIn] = tempFrame;
    can.rxIn = nextIn;
  }
}

// void CAN_Test01(void)
// {
//   uint8_t state = 0;
//   uint8_t txData1[8] = {0};
//   for(uint16_t i = 0; i < 8; i++){
//     txData1[i] = i + 1;
//   }
//   // uint8_t txData2[4] = {0x11,0x22,0x33,0x44};

//   CAN_TxHeader_t txHeader;
//   txHeader.stdId = HAL_GetTick() % 2048;
  
//   printf("id=%d\n",txHeader.stdId);

//   txHeader.rtr = CAN_RTR_DATA;
//   for(uint16_t i = 0; i < 10; i++){
//     state = CAN_Transmit(&txHeader,txData1,sizeof(txData1));
//     printf("state=%d\n",state);
//   }

//   // printf("state=%d\n",state);
// }

// void CAN_Test02(void)
// {
//   uint8_t state = 0;
//   // uint8_t txData2[4] = {0x11,0x22,0x33,0x44};

//   CAN_TxHeader_t txHeader;
//   txHeader.stdId = HAL_GetTick() % 2048;

//   printf("id=%d\n",txHeader.stdId);
//   txHeader.rtr = CAN_RTR_REMOTE;

//   state = CAN_Transmit(&txHeader,0,0);
//   printf("state=%d\n",state);
// }

// void HAL_CAN_RxFifo0FullCallback(CAN_HandleTypeDef *hcan)
// {
//   printf("full\n");
// }
