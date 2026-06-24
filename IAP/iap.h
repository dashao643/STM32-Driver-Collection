#ifndef __IAP_H__
#define __IAP_H__

#include "main.h"
#include "usart.h"
#include <stdbool.h>

#define BOOTLOADER_SIZE               0x4000                // 16KB

/********************* ↓选择IAP传输方式↓ *******************/
#define IAP_MODEL_FROM_UART           1                    // 从串口升级
#define IAP_MODEL_FROM_EXT_FLASH      2                    // 从SPI外部Flash升级

#define IAP_MODEL                     IAP_MODEL_FROM_UART  
/********************* ↑选择IAP传输方式↑ *******************/

#if IAP_MODEL == IAP_MODEL_FROM_UART 
#define IAP_INSTANCE                  USART1
#define IAP_HANDLE                    &huart1
#endif

#define IAP_TX_TIMEOUT                50
#define IAP_RX_TIMEOUT                1000
#define IAP_RX_BUFF_MAXLENTH          FLASH_PAGE_SIZE               // 数据包长度：一页大小：1024B
#define IAP_TX_LENTH                  1                             // 回复帧帧长

#define IAP_MAGIC_ADDR                0x20004FFC                    // 标志存储地址，RAM最后四个字节
#define IAP_MAGIC_VAL                 0xA5A5A5A5
#define IAP_APP_ADDR                  FLASH_BASE + BOOTLOADER_SIZE  // 0x08004000
#define IAP_ACK_BYTE                  0x79

#define IAP_ERROR_OVERFLOW            0xFF                          // 程序包溢出
#define IAP_ERROR_ERASE               0xFE                          // FLASH擦除错误
#define IAP_ERROR_APPSTACK            0xFD                          // APP程序栈指针错误

// 1 检查内存 + 发送ack
// 2 等包，收到包->3 收到过包+超时 ->4 没收到过包+超时->4 error
// 3 写入FLASH ->2
// 4 关闭时钟
// 5 执行跳转
typedef enum {
  IAP_SEND_ACK,
  IAP_WAIT_PACKAGE,
  IAP_WRITE_FLASH,
  IAP_FINISH,
} IAP_e;

bool IAP_RAM_Check(void);
void IAP_Run(void);
void IAP_JumpApp(void);

#endif
