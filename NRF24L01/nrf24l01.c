#include "nrf24l01.h"
#include "nrf24l01_def.h"
#include "general.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_def.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "gpio.h"

const uint8_t TX_ADDR[NRF24L01_ADDR_SIZE] = {0x11,0x22,0x33,0x44,0x55};
const uint8_t RX_ADDR[NRF24L01_ADDR_SIZE] = {0x11,0x22,0x33,0x44,0x55};

static uint32_t timer = 0;
static bool rxFlag = false;

static inline void CS_LOW(void);
static inline void CS_HIGH(void);
static inline void CE_LOW(void);
static inline void CE_HIGH(void);
static HAL_StatusTypeDef transmit(const uint8_t *data, uint16_t size);
static HAL_StatusTypeDef receive(uint8_t *data, uint16_t size);
static HAL_StatusTypeDef cmdTransmit(uint8_t cmd);

static void setPowerOn(void);
static void setPowerDown(void);
static void setTxMode(void);
static void setRxMode(void);
static void setTxRxAddr(uint8_t regAddr, const uint8_t *data);

static inline void CS_LOW(void) 
{
  HAL_GPIO_WritePin(SPI1_NRF24L01_CS_GPIO_Port, SPI1_NRF24L01_CS_Pin, GPIO_PIN_RESET);
}

static inline void CS_HIGH(void) 
{
  HAL_GPIO_WritePin(SPI1_NRF24L01_CS_GPIO_Port, SPI1_NRF24L01_CS_Pin, GPIO_PIN_SET);
}

static inline void CE_LOW(void)
{
  HAL_GPIO_WritePin(NRF24L01_CE_GPIO_Port, NRF24L01_CE_Pin, GPIO_PIN_RESET);
}

static inline void CE_HIGH(void)
{
  HAL_GPIO_WritePin(NRF24L01_CE_GPIO_Port, NRF24L01_CE_Pin, GPIO_PIN_SET);
}

static inline HAL_StatusTypeDef transmit(const uint8_t *data, uint16_t size) 
{
  return HAL_SPI_Transmit(NRF24L01_HANDLE, data, size, NRF24L01_TX_TIMEOUT_MS);
}

static inline HAL_StatusTypeDef receive(uint8_t *data, uint16_t size) 
{
  return HAL_SPI_Receive(NRF24L01_HANDLE, data, size, NRF24L01_TX_TIMEOUT_MS);
}

static HAL_StatusTypeDef cmdTransmit(uint8_t cmd)
{
  return transmit(&cmd, 1);
}

static void setPowerOn(void)
{
  uint8_t data = 0;
  NRF24L01_ReadReg(NRF24L01_ADDR_CONFIG, &data);
  data |= 0x02;
  NRF24L01_WriteReg(NRF24L01_ADDR_CONFIG, data);
}

static void setPowerDown(void)
{
  uint8_t data = 0;
  NRF24L01_ReadReg(NRF24L01_ADDR_CONFIG, &data);
  data &= 0xFD;
  NRF24L01_WriteReg(NRF24L01_ADDR_CONFIG, data);
}

static void setTxMode(void)
{
  uint8_t data = 0;
  NRF24L01_ReadReg(NRF24L01_ADDR_CONFIG, &data);
  data &= 0xFE; // 置0
  NRF24L01_WriteReg(NRF24L01_ADDR_CONFIG, data);
}

static void setRxMode(void)
{
  uint8_t data = 0;
  NRF24L01_ReadReg(NRF24L01_ADDR_CONFIG, &data);
  data |= 0x01; // 置1
  NRF24L01_WriteReg(NRF24L01_ADDR_CONFIG, data);
}

static void setTxRxAddr(uint8_t regAddr, const uint8_t *data)
{
  if(regAddr < NRF24L01_ADDR_RX_ADDR_P0 || regAddr > NRF24L01_ADDR_TX_ADDR) return;

  regAddr |= 0x20;
  CS_LOW();
  transmit(&regAddr, 1);
  transmit(data, NRF24L01_ADDR_SIZE);
  CS_HIGH();
}

// 读状态寄存器，读FIFO寄存器

void NRF24L01_Init(void)
{
  NRF24L01_WriteReg(NRF24L01_ADDR_CONFIG, NRF24L01_REG_CONFIG_TX);
  NRF24L01_WriteReg(NRF24L01_ADDR_EN_AA, NRF24L01_REG_EN_AA);
  NRF24L01_WriteReg(NRF24L01_ADDR_EN_RXADDR, NRF24L01_REG_EN_RXADDR);
  NRF24L01_WriteReg(NRF24L01_ADDR_SETUP_AW, NRF24L01_REG_SETUP_AW);
  NRF24L01_WriteReg(NRF24L01_ADDR_SETUP_RETR, NRF24L01_REG_SETUP_RETR);
  NRF24L01_WriteReg(NRF24L01_ADDR_RF_CH, NRF24L01_REG_RF_CH);
  NRF24L01_WriteReg(NRF24L01_ADDR_RF_SETUP, NRF24L01_REG_RF_SETUP);
  NRF24L01_WriteReg(NRF24L01_ADDR_RX_PW_P0, NRF24L01_REG_RX_PW_P0);

  // 设置发送地址
	setTxRxAddr(NRF24L01_ADDR_TX_ADDR, TX_ADDR);
  // 设置管道0的接收地址
	setTxRxAddr(NRF24L01_ADDR_RX_ADDR_P0, RX_ADDR);
}

HAL_StatusTypeDef NRF24L01_Transmit(uint8_t data)
{
  setPowerOn();
  
  CS_LOW();
  cmdTransmit(NRF24L01_CMD_W_TX_PAYLOAD);
  uint8_t state = transmit(&data, NRF24L01_DATA_SIZE);
  CS_HIGH();
  if(state != HAL_OK) return state;
  
  setTxMode();
  CE_HIGH();
  // Delay_us(200);
  HAL_Delay(1);
  CE_LOW();

  timer = HAL_GetTick();
  // 发送完成后 等待STATUS寄存器ack标志
  uint8_t regVal;
  bool noAck = false;
  do{
    NRF24L01_ReadReg(NRF24L01_ADDR_STATUS, &regVal);

    // if((HAL_GetTick() - timer) > NRF24L01_WAIT_ACK_MS){
    //   noAck = true;
    //   break;
    // }

    // 达到最大重传次数
    if(regVal & 0x10){
      noAck = true;
      break;
    }
  // 第5位寄存器被置1，跳出等待
  }while(!(regVal & 0x20));

  if(noAck){
    // 清除重传标志
    NRF24L01_ClearMaxRT();
    printf("tx fail\n");
  }
  // 清除ack标志
  else{
    NRF24L01_ClearTxAck();
  }
  NRF24L01_FlushTx();

  return HAL_OK;
}

HAL_StatusTypeDef NRF24L01_Receive(uint8_t *data)
{
  CS_LOW();
  cmdTransmit(NRF24L01_CMD_R_RX_PAYLOAD);
  uint8_t state = receive(data, 1);
  CS_HIGH();

  return state;
}

HAL_StatusTypeDef NRF24L01_WriteReg(uint8_t regAddr, uint8_t data)
{
  if(regAddr > NRF24L01_ADDR_FEATURE) return HAL_ERROR;

  uint8_t arr[2] = {regAddr | 0x20, data};
  CS_LOW();
  uint8_t state = transmit(arr, 2);
  CS_HIGH();

  return state;
}

HAL_StatusTypeDef NRF24L01_ReadReg(uint8_t regAddr, uint8_t *data)
{
  if(regAddr > NRF24L01_ADDR_FEATURE) return HAL_ERROR;

  uint8_t state;
  CS_LOW();
  state = transmit(&regAddr, 1);
  if(state != HAL_OK) return state;

  state = receive(data, 1);
  CS_HIGH();

  return state;
}

void NRF24L01_FlushTx(void)
{
  CS_LOW();
  cmdTransmit(NRF24L01_CMD_FLUSH_TX);
  CS_HIGH();
}

void NRF24L01_FlushRx(void)
{
  CS_LOW();
  cmdTransmit(NRF24L01_CMD_FLUSH_RX);
  CS_HIGH();
}

void NRF24L01_ClearRxIrq(void)
{
  uint8_t data = 0;
  NRF24L01_ReadReg(NRF24L01_ADDR_STATUS, &data);
  data |= 0x40;
  NRF24L01_WriteReg(NRF24L01_ADDR_STATUS, data);
}

void NRF24L01_ClearTxAck(void)
{
  uint8_t data = 0;
  NRF24L01_ReadReg(NRF24L01_ADDR_STATUS, &data);
  data |= 0x20;
  NRF24L01_WriteReg(NRF24L01_ADDR_STATUS, data);
}

void NRF24L01_ClearMaxRT(void)
{
  uint8_t data = 0;
  NRF24L01_ReadReg(NRF24L01_ADDR_STATUS, &data);
  data |= 0x10;
  NRF24L01_WriteReg(NRF24L01_ADDR_STATUS, data);
}

// 读取地址
void NRF24L01_test(void)
{
  // uint8_t data[5] = {0};
  // uint8_t addr = NRF24L01_ADDR_TX_ADDR;
  // CS_LOW();
  // transmit(&addr, 1);
  // receive(data, 5);
  // CS_HIGH();
  // for(uint8_t i = 0; i < 5; i++){
  //   printf("%d ",data[i]);
  // }
  // printf("\n");

  setPowerOn();
}

void NRF24L01_Task(void)
{
  if(rxFlag){
    rxFlag = false;
    // 读出数据
    uint8_t data;
    NRF24L01_Receive(&data);

  }
}

void NRF24L01_SetRxFlag(void)
{
  rxFlag = true;
  NRF24L01_ClearRxIrq();
}