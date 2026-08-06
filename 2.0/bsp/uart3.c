#include "uart3.h"


// 绕回缓冲区
static void idleProcess(void)
{
    if (__HAL_UART_GET_FLAG(&uart1, UART_FLAG_IDLE) == SET) {
        __HAL_UART_CLEAR_IDLEFLAG(&uart1);

        uint16_t dmaRemain = __HAL_DMA_GET_COUNTER(uart1.hdmarx);
        rx->rxSize = segmentSize - remaining;

    }
}

void UART3_Init(void)
{

}

void UART3_MspInit(UART_HandleTypeDef *huart)
{

}

HAL_StatusTypeDef UART3_Transmit(const uint8_t *data, uint16_t size)
{

}
HAL_StatusTypeDef UART3_Transmit_DMA(const uint8_t *data, uint16_t size)
{

}

UART_HandleTypeDef* UART3_GetHandle(void)
{

}

UART3_RxTypeDef* UART3_GetRxStruct(void)
{

}