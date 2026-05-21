#ifndef __NRF24L01_H__
#define __NRF24L01_H__

#include "nrf24l01_def.h"
#include "spi.h"
#include <stdint.h>
#include <stdbool.h>

/********************* ↓选择主机/从机↓ *******************/
// #define NRF24L01_MASTER
#define NRF24L01_SLAVE
/********************* ↑选择主机/从机↑ *******************/

#define NRF24L01_INSTANCE                 SPI1
#define NRF24L01_HANDLE                   &hspi1
#define NRF24L01_TX_TIMEOUT_MS            100
#define NRF24L01_WAIT_ACK_MS              10
#define NRF24L01_ADDR_SIZE                5
#define NRF24L01_DATA_SIZE                1                 // 固定数据长度

#define NRF24L01_REG_CONFIG_PWR_UP        0x0A              // 仅上电
#define NRF24L01_REG_CONFIG_TX            0x0A              // 上电 + PTX
#define NRF24L01_REG_CONFIG_RX            0x0B              // 上电 + PRX
#define NRF24L01_REG_EN_AA                0x3F              // 所有通道启用自动确认
#define NRF24L01_REG_EN_RXADDR            0x01              // 启用rx管道0
#define NRF24L01_REG_SETUP_AW             0x03              // 5字节地址
#define NRF24L01_REG_SETUP_RETR           0x03              // 自动重传间隔：250us，重传次数：3次
#define NRF24L01_REG_RF_CH                0x02              // 设置工作频率通道
#define NRF24L01_REG_RF_SETUP             0x0E              // 设置无限传输频率和功率：2Mbps 0dBm

#define NRF24L01_REG_RX_PW_P0             NRF24L01_DATA_SIZE   // rx管道0接收字节长度:1-32

typedef struct {
  uint8_t rxbuf[NRF24L01_DATA_SIZE];
  bool rxFlag;
} NRF24L01_t;

void NRF24L01_Init(void);
void NRF24L01_Task(void);
void NRF24L01_SetRxFlag(void);

HAL_StatusTypeDef NRF24L01_Transmit(const uint8_t *data, uint8_t size);
HAL_StatusTypeDef NRF24L01_Receive(uint8_t *data, uint8_t size);
HAL_StatusTypeDef NRF24L01_WriteReg(uint8_t regAddr, uint8_t data);
HAL_StatusTypeDef NRF24L01_ReadReg(uint8_t regAddr, uint8_t *data);

#endif
