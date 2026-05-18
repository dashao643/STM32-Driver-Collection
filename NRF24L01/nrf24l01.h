#ifndef __NRF24L01_MASTER_H__
#define __NRF24L01_MASTER_H__

#include "spi.h"
#include <stdint.h>

#define NRF24L01_INSTANCE                 SPI1
#define NRF24L01_HANDLE                   &hspi1
#define NRF24L01_TX_TIMEOUT_MS            100
#define NRF24L01_WAIT_ACK_MS              10

#define NRF24L01_DATA_SIZE                1                 // 数据长度固定1字节

#define NRF24L01_CMD_R_RX_PAYLOAD         0x61              // 读取RX数据 1-32B 低字节优先
#define NRF24L01_CMD_W_TX_PAYLOAD         0xA0              // 写入TX数据 1-32B
#define NRF24L01_CMD_FLUSH_TX             0xE1
#define NRF24L01_CMD_FLUSH_RX             0xE2
#define NRF24L01_CMD_R_RX_PL_WID          0x60              // 读取RX字节大小
#define NRF24L01_CMD_NO_ACK               0xB0

#define NRF24L01_REG_ADDR_CONFIG          0x00
#define NRF24L01_REG_POWER_PTX            0x0A              // 上电 + PTX

#define NRF24L01_REG_ADDR_STATUS          0x07
#define NRF24L01_REG_RX_DR                0x40              // RX数据到达
#define NRF24L01_REG_TX_DS                0x20              // TX发送完成
#define NRF24L01_REG_MAX_RT               0x10              // 达到最大重传

#define NRF24L01_REG_ADDR_RX_ADDR         0x0A              // 接收地址的数据管道地址
#define NRF24L01_REG_RX_ADDR_P0           0xE7E7E7E7E7      // 管道0地址

void NRF24L01_Init(void);
HAL_StatusTypeDef NRF24L01_Transmit(uint8_t data);
void NRF24L01_ReadReg(uint8_t *data);

#endif
