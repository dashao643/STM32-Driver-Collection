#include "my_uart.h"

#include <stdint.h>
#include <string.h>

// 后期改成不调用memset
void UART_Clear(My_UART_t *uart)
{
  memset(uart->rxBuf, 0, uart->rxMaxSize);

  uart->rxSize = 0;
  uart->frameEnd = false;
}

void UART_IdleProcess(const USART_TypeDef* Instance, My_UART_t *uart)
{
  if(Instance != uart->instance)  
    return;

  if(__HAL_UART_GET_FLAG(uart->handle, UART_FLAG_IDLE) == SET){
      __HAL_UART_CLEAR_IDLEFLAG(uart->handle);

    // 计算收到的字节数                  剩余字节大小
    uart->rxSize = uart->rxMaxSize - __HAL_DMA_GET_COUNTER(uart->handle->hdmarx);

    uart->frameEnd = true;

    HAL_UART_DMAStop(uart->handle);
    HAL_UART_Receive_DMA(uart->handle, uart->rxBuf, uart->rxMaxSize);
  }
}

void UART_Clear_AT(My_UART_t *uart)
{
  uart->rxSize = 0;
  uart->rxIdx = 0;
  uart->frameEnd = false;

  HAL_UART_DMAStop(uart->handle);
  // 处理完成，指针回到开头
  HAL_UART_Receive_DMA(uart->handle, uart->rxBuf, uart->rxMaxSize);
}

// 适配AT指令的绕回缓冲区,数据连续到达时，向后追加,(数据空间为 0 到 uart->rxIdx - 1)
void UART_IdleProcess_AT(const USART_TypeDef* Instance, My_UART_t *uart)
{
  if(Instance != uart->instance)
    return;

  if(__HAL_UART_GET_FLAG(uart->handle, UART_FLAG_IDLE) == SET){
    __HAL_UART_CLEAR_IDLEFLAG(uart->handle);

    // 计算本次收到字节数（相对于当前DMA段的剩余长度）
    uint16_t dmaRemain = __HAL_DMA_GET_COUNTER(uart->handle->hdmarx);

    uint16_t segmentSize = uart->rxMaxSize - uart->rxIdx;
    
    uart->rxSize = segmentSize - remaining;
    uart->rxIdx += uart->rxSize;

    // 指针越界时绕回
    if(uart->rxIdx >= uart->rxMaxSize)
      uart->rxIdx = 0;

    uart->frameEnd = true;

    HAL_UART_DMAStop(uart->handle);
    HAL_UART_Receive_DMA(uart->handle, uart->rxBuf + uart->rxIdx, uart->rxMaxSize - uart->rxIdx);
  }
}

/**
 * @brief 串口发送函数
 * 
 * @param uart My_UART_t结构体指针
 * @param size 发送的数据大小，最大不超过tx数组大小
 * @param mode 发送模式：阻塞式，DMA式
 * @param timeout 阻塞式的超时时间，DMA填0
 */
void UART_Transmit(const My_UART_t* uart, uint16_t size, TransmitMode_e mode, uint32_t timeout)
{
  if(size > uart->txMaxSize)
    size = uart->txMaxSize;

  if(mode == BLOCK)
    HAL_UART_Transmit(uart->handle, uart->txBuf, size, timeout);

  else if(mode == DMA){
    HAL_UART_Transmit_DMA(uart->handle, uart->txBuf, size);
  }
}
