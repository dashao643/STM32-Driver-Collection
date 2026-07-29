#include "my_can.h"
#include "stm32f1xx_hal.h"
#include "can_app.h"
#include "stm32f1xx_hal_can.h"

#include "stdio.h"
#ifdef MODBUS_DEBUG
#include "modbus.h"
#endif

/*
APB1时钟36Mhz 
PSC: 6
BS1(8) + BS2(3) : 11(8 - 16)
SJW: 1
bsp = 36 000 000 / 6 / (11 + 1)

屏蔽位值为1，表示要匹配;
为0表示不需要匹配

标准ID 扩展ID ID类型 帧类型 
过滤器映像:11位STID + 18位EXID + IDE + RTR + 固定0 = 32
*/

static CAN_RxQueue_t can = {0};
static uint32_t txTimer = 0;

static void setFilter(void);
static void frameExecute(const CAN_Rx_t* frame);
static void frameReply(uint16_t stdId);
static HAL_StatusTypeDef transmit(const CAN_TxHeaderTypeDef *pHeader, const uint8_t aData[]);

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
  // filter.FilterMaskIdHigh = 0xF000;
  filter.FilterMaskIdHigh = 0x0000;
  filter.FilterMaskIdLow  = 0x0000;

  filter.FilterActivation = CAN_FILTER_ENABLE;
  filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  filter.SlaveStartFilterBank = 0;
  
  HAL_CAN_ConfigFilter(CAN_HANDLE, &filter);
}

// 调用应用层函数执行对应操作
static void frameExecute(const CAN_Rx_t* frame)
{
#ifdef PRINT_DEBUG
  printf("IDE:%d\n",    (int)frame->rxHeader.IDE);
  printf("StdId:%d\n",  (int)frame->rxHeader.StdId);
  printf("DLC:%d\n",    (int)frame->rxHeader.DLC);
  printf("RTR:%d\n",    (int)frame->rxHeader.RTR);
  printf("receive data frame\n");
  for(uint8_t i = 0; i < frame->rxHeader.DLC; i++){
    printf("data[%d]=%d ",i,frame->data[i]);
  }
  bool res = CAN_APP_DataFrame(frame->rxHeader.StdId, frame->data, frame->rxHeader.DLC);
  printf("res=%d\n", res);
#endif
#ifdef MODBUS_DEBUG
  Modbus_Transmit((uint8_t*)&frame->rxHeader.IDE, 1);
  Modbus_Transmit((uint8_t*)&frame->rxHeader.StdId, 1);
  Modbus_Transmit((uint8_t*)&frame->rxHeader.DLC, 1);
  Modbus_Transmit((uint8_t*)&frame->rxHeader.RTR, 1);
  Modbus_Transmit((uint8_t*)frame->data, frame->rxHeader.DLC);
  bool res = CAN_APP_DataFrame(frame->rxHeader.StdId, frame->data, frame->rxHeader.DLC);
  Modbus_Transmit((uint8_t*)&res, 1);
#endif
}

// 调用应用层函数回复帧
static void frameReply(uint16_t stdId)
{
#ifdef PRINT_DEBUG
  printf("StdId:%d\n",  (int)stdId);
  printf("receive remote frame\n");
  uint8_t state = CAN_APP_RemoteFrame(stdId);
  printf("state=%d\n", state);
#endif
#ifdef MODBUS_DEBUG
  Modbus_Transmit((uint8_t*)&stdId, 1);
  uint8_t state = CAN_APP_RemoteFrame(stdId);
  Modbus_Transmit(&state, 1);
#endif
}

static HAL_StatusTypeDef transmit(const CAN_TxHeaderTypeDef *pHeader, const uint8_t aData[])
{
  // 等待可用的发送邮箱
  txTimer = HAL_GetTick();
  while(HAL_CAN_GetTxMailboxesFreeLevel(CAN_HANDLE) == 0){
    if((HAL_GetTick() - txTimer) > CAN_TX_TIMEOUT_MS)
      return HAL_TIMEOUT;
  }
  // 返回实际使用的邮箱编号
  uint32_t txMailbox;
  uint8_t state;
  state = HAL_CAN_AddTxMessage(CAN_HANDLE, pHeader, aData, &txMailbox);
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

void CAN_Init(void)
{
  setFilter();
  HAL_CAN_Start(CAN_HANDLE);
  // 开启can接收中断
  HAL_CAN_ActivateNotification(CAN_HANDLE, CAN_IT_RX_FIFO0_MSG_PENDING);
}

void CAN_Task(void)
{
  // 两个指针相等表示队列为空
  if(can.rxIn == can.rxOut){
    return;
  }
  // 解析数据
  while(can.rxIn != can.rxOut){
    CAN_Rx_t frame = can.rxQueue[can.rxOut];

    // 接收到数据帧,显示数据并执行
    if(frame.rxHeader.RTR == CAN_RTR_DATA){
      frameExecute(&frame);
    }
    // 接收到遥控帧,回传数据
    else {
      frameReply(frame.rxHeader.StdId);
    }
    can.rxOut = (can.rxOut + 1) % CAN_RX_QUEUE_NUM;
  }
}

/**
 * @brief CAN数据帧发送函数
 * 
 * @param txHeader 数据头结构体
 * @param data 数组指针
 * @param size 数据大小
 * @return HAL_StatusTypeDef 返回状态
 */
HAL_StatusTypeDef CAN_SendDataFrame(uint16_t stdId, const uint8_t *data, uint8_t size)
{
  if(stdId > CAN_STD_ID_MAX) return HAL_ERROR;
  if(size == 0 || size > CAN_DATA_SIZE_MAX) return HAL_ERROR;

  CAN_TxHeaderTypeDef TxHeader = {0};

  TxHeader.IDE = CAN_ID_STD;                      // 指定ID类型
  TxHeader.StdId = stdId;                         // 配置ID
  TxHeader.RTR = CAN_RTR_DATA;                    // 指定帧类型
  TxHeader.DLC = size;                            // 数据长度 0 - 8
  TxHeader.TransmitGlobalTime = DISABLE;          // 时间戳设置

  return transmit(&TxHeader, data);
}

HAL_StatusTypeDef CAN_SendRemoteFrame(uint16_t stdId)
{
  if(stdId > CAN_STD_ID_MAX) return HAL_ERROR;

  CAN_TxHeaderTypeDef TxHeader = {0};

  TxHeader.IDE = CAN_ID_STD;                      // 指定ID类型
  TxHeader.StdId = stdId;                         // 配置ID
  TxHeader.RTR = CAN_RTR_REMOTE;                  // 指定帧类型
  TxHeader.DLC = 0;                               // 数据长度 0 - 8
  TxHeader.TransmitGlobalTime = DISABLE;          // 时间戳设置

  return transmit(&TxHeader, NULL);
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
      return;
    }
    can.rxQueue[can.rxIn] = tempFrame;
    can.rxIn = nextIn;
  }
}

void CAN_Debug(void)
{
  uint32_t msr = CAN1->MSR;
  uint32_t esr = CAN1->ESR;
  uint32_t tsr = CAN1->TSR;

  printf("=== CAN Debug ===\r\n");
  printf("MSR=%08lx  ", msr);
  if (msr & CAN_MSR_INAK)  printf("INAK ");
  if (msr & CAN_MSR_SLAK)  printf("SLAK ");
  if (msr & CAN_MSR_ERRI)  printf("ERRI ");
  if (msr & CAN_MSR_SLAKI) printf("SLAKI ");
  if (msr & CAN_MSR_WKUI)  printf("WKUI ");
  if (msr & CAN_MSR_TXM)   printf("TXM ");
  if (msr & CAN_MSR_RXM)   printf("RXM ");
  // if (msr & CAN_MSR_RXF)   printf("RXF ");
  printf("\r\n");

  uint32_t lec = (esr & CAN_ESR_LEC) >> CAN_ESR_LEC_Pos;
  printf("ESR=%08lx  TEC=%lu REC=%lu LEC=", esr,
         (esr & CAN_ESR_TEC) >> CAN_ESR_TEC_Pos,
         (esr & CAN_ESR_REC) >> CAN_ESR_REC_Pos);
  switch (lec) {
    case 0: printf("NoErr"); break;
    case 1: printf("Stuff"); break;
    case 2: printf("Form"); break;
    case 3: printf("Ack"); break;
    case 4: printf("Recessive"); break;
    case 5: printf("Dominant"); break;
    case 6: printf("CRC"); break;
    case 7: printf("Custom"); break;
    default: printf("?"); break;
  }
  if (esr & CAN_ESR_BOFF) printf(" BOFF!");
  if (esr & CAN_ESR_EPVF) printf(" EPVF!");
  if (esr & CAN_ESR_EWGF) printf(" EWGF!");
  printf("\r\n");

  printf("TSR=%08lx  TME=%d%d%d  ", tsr,
         (tsr & CAN_TSR_TME0) ? 1 : 0,
         (tsr & CAN_TSR_TME1) ? 1 : 0,
         (tsr & CAN_TSR_TME2) ? 1 : 0);
  printf("RQCP=%d%d%d\r\n",
         (tsr & CAN_TSR_RQCP0) ? 1 : 0,
         (tsr & CAN_TSR_RQCP1) ? 1 : 0,
         (tsr & CAN_TSR_RQCP2) ? 1 : 0);
}
