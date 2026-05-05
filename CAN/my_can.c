#include "my_can.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_can.h"
#include "stm32f1xx_hal_def.h"
#include <stdint.h>
#include <stdio.h>

// 屏蔽位值为1，表示要匹配;
// 为0表示不需要匹配

// 映像:11位STID + 18位EXID + IDE + RTR + 0 = 32

HAL_StatusTypeDef CAN_SetFilter(void)
{
  CAN_FilterTypeDef filter;
  // 过滤器组
  filter.FilterBank = 0;
  // 过滤器模式
  filter.FilterMode = CAN_FILTERMODE_IDMASK;
  // 32位长度
  filter.FilterScale = CAN_FILTERSCALE_32BIT;
  
  // 全接收
  filter.FilterIdHigh = 0x0000;
  filter.FilterIdLow = 0x0000;
  filter.FilterMaskIdHigh = 0x0000;
  filter.FilterMaskIdLow = 0x0000;

  // 只收奇数
  // filter.FilterIdHigh = 0x0020;
  // filter.FilterIdLow = 0x0000;
  // filter.FilterMaskIdHigh = 0x0020;
  // filter.FilterMaskIdLow = 0x0000;

  // 启动过滤器
  filter.FilterActivation = CAN_FILTER_ENABLE;
  // 选择FIFO
  filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  // 单CAN无效
  filter.SlaveStartFilterBank = 0;

  return HAL_CAN_ConfigFilter(&hcan, &filter);
}

/**
 * @brief 
 * 
 * @param stdID 0 - 2047
 * @param data 
 * @param size 数据长度（数据帧：1~8，遥控帧：0）
 * @param frameType CAN_RTR_DATA / CAN_RTR_REMOTE
 */
void CAN_TestPoll(uint16_t stdID, uint8_t *data, uint8_t size, uint8_t frameType)
{
  if((frameType != CAN_RTR_DATA) && (frameType != CAN_RTR_REMOTE)) return;
  if((frameType == CAN_RTR_DATA) && (size == 0)) return;
  if(size > 8)  return;

  CAN_TxHeaderTypeDef TxHeader = {0};

  // 指定ID类型
  TxHeader.IDE = CAN_ID_STD;
  // 配置ID
  TxHeader.StdId = stdID;
  // 指定帧类型
  TxHeader.RTR = frameType;
  // 数据长度 0 - 8
  TxHeader.DLC = size;
  // 时间戳设置
  TxHeader.TransmitGlobalTime = DISABLE;
  
  // 死等可用的发送邮箱
  while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan) < 1);
  // 返回实际使用的邮箱编号
  uint32_t TxMailbox;
  uint8_t state = HAL_CAN_AddTxMessage(&hcan, &TxHeader, data, &TxMailbox);
  if(state != HAL_OK){
    printf("can tx error:%d\n",state);
    return;
  }
// ====================== 等待当前帧发送完成 ======================
  while (HAL_CAN_IsTxMessagePending(&hcan, TxMailbox));




  // res: 0 - 3
  if (HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0) < 1){
    printf("can rx no msg\n");
    return;
  }  
  uint8_t RxData[8];
  CAN_RxHeaderTypeDef rxHeader;
  HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &rxHeader, RxData);
  printf("IDE:%d\n",(int)rxHeader.IDE);
  printf("StdId:%d\n",(int)rxHeader.StdId);
  printf("DLC:%d\n",(int)rxHeader.DLC);
  printf("RTR:%d\n",(int)rxHeader.RTR);
  // 接收到了数据帧,显示数据
  if(rxHeader.RTR == CAN_RTR_DATA){
    for(uint8_t i = 0; i < rxHeader.DLC; i++){
      printf("data[%d]=%d\n",i,RxData[i]);
    }
  }
  // 接收到了遥控帧,回传数据,等会实现
  else {

  }
}