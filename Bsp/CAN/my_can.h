#ifndef __MY_CAN_H__
#define __MY_CAN_H__

#include "can.h"
#include "can_app.h"
#include <stdint.h>
#include <stdbool.h>

// #define PRINT_DEBUG
#define MODBUS_DEBUG

#define CAN_INSTANCE                  CAN1
#define CAN_HANDLE                    &hcan
#define CAN_STD_ID_MAX                2047
#define CAN_DATA_SIZE_MAX             8
#define CAN_TX_TIMEOUT_MS             50
#define CAN_RX_QUEUE_NUM              11   // 接收队列数量,实际容量为10

#define CAN_MASK_MIN                  0
#define CAN_MASK_MAX                  127

// 目前只支持标准id

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

void CAN_Debug(void);

void CAN_Init(void);
void CAN_Task(void);
HAL_StatusTypeDef CAN_SendDataFrame(uint16_t stdId, const uint8_t *data, uint8_t size);
HAL_StatusTypeDef CAN_SendRemoteFrame(uint16_t stdId);

#endif
