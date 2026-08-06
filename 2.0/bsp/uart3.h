#ifndef __UART3_H__
#define __UART3_H__

#include "stm32f1xx_hal.h"

#include <stdbool.h>

#define UART3_RX_BUFF_MAX_SIZE               256     // 接收最大帧长度
#define UART3_TX_BUFF_MAX_SIZE               256     // 发送最大帧长度
#define UART3_TX_TIMEOUT_MS                  500

typedef struct {
    volatile bool rxFlag;                       // 接收到数据标志
    uint8_t rxBuf[UART3_RX_BUFF_MAX_SIZE];      // 接收缓冲区
    volatile uint16_t rxTail;                   // 缓冲区尾指针
} UART3_RxTypeDef;

void UART3_Init(void);
void UART3_MspInit(UART_HandleTypeDef *huart);

HAL_StatusTypeDef UART3_Transmit(const uint8_t *data, uint16_t size);
HAL_StatusTypeDef UART3_Transmit_DMA(const uint8_t *data, uint16_t size);

UART_HandleTypeDef* UART3_GetHandle(void);
UART3_RxTypeDef* UART3_GetRxStruct(void);

#endif
