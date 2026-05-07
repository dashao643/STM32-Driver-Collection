#ifndef __MY_CAN_H__
#define __MY_CAN_H__

#include "can.h"
#include <stdint.h>
#include <stdbool.h>

#define CAN_INSTANCE                  CAN1
#define CAN_HANDLE                    &hcan
#define CAN_STD_ID_MAX                2047
#define CAN_DATA_SIZE_MAX             8
#define CAN_TX_TIMEOUT_MS             50
#define CAN_RX_QUEUE_NUM              11   // 接收队列数量,实际容量为10

#define CAN_MASK_MIN                  0
#define CAN_MASK_MAX                  127

typedef struct
{
  // uint8_t ide;                    
  uint16_t stdId;                    // 0 - 2047
  uint8_t rtr;                       // CAN_RTR_DATA / CAN_RTR_REMOTE
}CAN_TxHeader_t;

typedef struct
{
  CAN_RxHeaderTypeDef rxHeader;
  uint8_t data[CAN_DATA_SIZE_MAX];
}CAN_Rx_t;

typedef struct
{
  CAN_Rx_t rxQueue[CAN_RX_QUEUE_NUM];
  uint8_t rxIn;
  uint8_t rxOut;
}CAN_RxQueue_t;

void CAN_Init(void);
void CAN_Task(void);
HAL_StatusTypeDef CAN_Transmit(const CAN_TxHeader_t *txHeader, const uint8_t *data, uint8_t size);

#endif
