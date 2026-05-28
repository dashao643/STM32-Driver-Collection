#include "stm32f1xx_hal.h"
#include "esp8266.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>

static uint8_t rxDataBuf[ESP8266_RX_DATA_MAXLENTH] = {0};
static uint8_t rxAtBuf[ESP8266_RX_AT_MAXLENTH] = {0};
// static uint8_t txBuf[ESP8266_TX_BUFF_MAXLENTH] = {0};
static ESP8266_t esp8266 = {0};

void ESP8266_AT_Transmit(const char *cmd)
{
  uint16_t len = strlen(cmd);

  HAL_UART_Transmit(ESP8266_HANDLE, (uint8_t*)cmd, len, ESP8266_TX_TIMEOUT_MS);
}

/*
问题根源：DMA 和阻塞式 HAL 函数冲突。

  看 usart.c，USART2 配置了 DMA（hdma_usart2_rx 和 hdma_usart2_tx），并且开启了 USART2 中断。

  但你在 esp8266.c 里用的是 HAL_UART_Transmit（阻塞）和 HAL_UART_Receive（阻塞）。

  HAL 阻塞函数在 handle 已经关联了 DMA 的情况下，行为是未定义的。 具体来说：

  - HAL_UART_Transmit 阻塞发送时，如果 DMA 中断触发，HAL 内部状态机会被打乱，huart2 的 gState 可能变成 BUSY
  - 下一次调用 HAL_UART_Receive 时，因为 handle 状态是 BUSY，直接返回 HAL_BUSY，不接收任何数据，rxAtBuf 保持上次内容
  - 而上次内容恰好是你发出去的 "123\r\n" 被回环或者 DMA 搬运过来的残留

  你看到的 abcAAT O 就是这种状态混乱下读到的垃圾数据。

  解决方案有两个：

  方案一（推荐）：既然配了 DMA，就用 DMA 方式收发

  // 发送改用 DMA
  HAL_UART_Transmit_DMA(ESP8266_HANDLE, (uint8_t*)cmd, len);
  // 等待发送完成
  while(HAL_UART_GetState(ESP8266_HANDLE) & HAL_UART_STATE_BUSY_TX);

  // 接收改用 DMA + 空闲中断
  HAL_UARTEx_ReceiveToIdle_DMA(ESP8266_HANDLE, rxAtBuf, len);
  // 等待接收完成（通过回调或标志位）

  方案二（简单）：在 CubeMX 里把 USART2 的 DMA 去掉，只保留中断，然后阻塞收发就能正常工作了。

  你现在的代码是阻塞式逻辑，但底层配了 DMA，两者打架了。最简单的修法是方案二——去掉 USART2 的 DMA
  配置，阻塞收发立刻就能用。
*/
void ESP8266_AT_Receive(uint8_t len)
{
  if(len > ESP8266_RX_AT_MAXLENTH) len = ESP8266_RX_AT_MAXLENTH;

  memset(rxAtBuf, 0, ESP8266_RX_AT_MAXLENTH);
  HAL_UART_Receive(ESP8266_HANDLE, rxAtBuf, len, ESP8266_RX_TIMEOUT_MS);
}

void ESP8266_Init(void)
{
  HAL_Delay(100);
  ESP8266_AT_Transmit("AT+RST\r\n");
  HAL_Delay(1);
  ESP8266_AT_Transmit("AT\r\n");
  // __HAL_UART_FLUSH_DRREGISTER(ESP8266_HANDLE);
  ESP8266_AT_Receive(8);
  
  printf("abc%s\n", rxAtBuf);
}

void ESP8266_Task(void)
{

}

My_UART_t* ESP8266_Get_UART(void)
{
  return &esp8266.uart;
}


