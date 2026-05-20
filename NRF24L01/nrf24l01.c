#include "nrf24l01.h"
#include "nrf24l01_def.h"
#include "general.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_def.h"

#include <stdio.h>

#include "led.h"
#include "modbus.h"
#include "oled.h"

const uint8_t TX_ADDR[NRF24L01_ADDR_SIZE] = {0x11,0x22,0x33,0x44,0x55};
const uint8_t RX_ADDR[NRF24L01_ADDR_SIZE] = {0x11,0x22,0x33,0x44,0x55};

static NRF24L01_t nrf24l01 = {0};

static inline void CS_LOW(void);
static inline void CS_HIGH(void);
static inline void CE_LOW(void);
static inline void CE_HIGH(void);
static HAL_StatusTypeDef transmit(const uint8_t *data, uint16_t size);
static HAL_StatusTypeDef receive(uint8_t *data, uint16_t size);
static HAL_StatusTypeDef cmdTransmit(uint8_t cmd);
static void setPowerOn(void) UNUSED_FUNC;
static void setPowerDown(void) UNUSED_FUNC;
static void setTxMode(void) UNUSED_FUNC;
static void setRxMode(void) UNUSED_FUNC;
static void setTxRxAddr(uint8_t regAddr, const uint8_t *data);
static void flushTx(void);
static void flushRx(void) UNUSED_FUNC;
static void clearRxIrq(void);
static void clearTxAck(void);
static void clearMaxRT(void);

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

// 设置Tx或Rx地址
static void setTxRxAddr(uint8_t regAddr, const uint8_t *data)
{
  if(regAddr < NRF24L01_ADDR_RX_ADDR_P0 || regAddr > NRF24L01_ADDR_TX_ADDR) return;

  regAddr |= 0x20;
  CS_LOW();
  transmit(&regAddr, 1);
  transmit(data, NRF24L01_ADDR_SIZE);
  CS_HIGH();
}

// 清除TxFIFO
static void flushTx(void)
{
  CS_LOW();
  cmdTransmit(NRF24L01_CMD_FLUSH_TX);
  CS_HIGH();
}

// 清除RxFIFO
static void flushRx(void)
{
  CS_LOW();
  cmdTransmit(NRF24L01_CMD_FLUSH_RX);
  CS_HIGH();
}

static void clearRxIrq(void)
{
  uint8_t data = 0;
  NRF24L01_ReadReg(NRF24L01_ADDR_STATUS, &data);
  data |= 0x40;
  NRF24L01_WriteReg(NRF24L01_ADDR_STATUS, data);
}

static void clearTxAck(void)
{
  uint8_t data = 0;
  NRF24L01_ReadReg(NRF24L01_ADDR_STATUS, &data);
  data |= 0x20;
  NRF24L01_WriteReg(NRF24L01_ADDR_STATUS, data);
}

// 清除最大重传次数标志
static void clearMaxRT(void)
{
  uint8_t data = 0;
  NRF24L01_ReadReg(NRF24L01_ADDR_STATUS, &data);
  data |= 0x10;
  NRF24L01_WriteReg(NRF24L01_ADDR_STATUS, data);
}

/*-----------------------------------------------------------------*/

/**
 * @brief NRF24L01发送函数
 * 
 * @param data 传输的字节数组
 * @param size 数据大小（必须和接收端定义的数据大小一致）
 * @return HAL_StatusTypeDef 返回状态
 */
HAL_StatusTypeDef NRF24L01_Transmit(const uint8_t *data, uint8_t size)
{
  if(size != NRF24L01_DATA_SIZE) return HAL_ERROR;
  if(size > 32) return HAL_ERROR;

  CS_LOW();
  cmdTransmit(NRF24L01_CMD_W_TX_PAYLOAD);
  uint8_t state = transmit(data, size);
  CS_HIGH();
  if(state != HAL_OK) return state;

  CE_HIGH();
  Delay_us(20);
  CE_LOW();

  // 发送完成后 等待STATUS寄存器ack标志
  uint8_t regVal;
  bool noAck = false;
  uint32_t timer = HAL_GetTick();
  do{
    NRF24L01_ReadReg(NRF24L01_ADDR_STATUS, &regVal);
    // 达到最大重传次数
    if(regVal & 0x10){
      noAck = true;
      break;
    }
    if((HAL_GetTick() - timer) > NRF24L01_WAIT_ACK_MS){
      noAck = true;
      break;
    }
  // 第5位寄存器被置1，跳出等待
  }while(!(regVal & 0x20));

  if(noAck){
    clearMaxRT();
    printf("tx fail\n");
  }
  else{
    clearTxAck();
  }
  flushTx();

  return HAL_OK;
}

HAL_StatusTypeDef NRF24L01_Receive(uint8_t *data, uint8_t size)
{
  if(size == 0 || size > 32) return HAL_ERROR;

  CS_LOW();
  cmdTransmit(NRF24L01_CMD_R_RX_PAYLOAD);
  uint8_t state = receive(data, size);
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

void NRF24L01_Init(void)
{
  // 设定接收模式有时设置失败，先彻底掉电
  NRF24L01_WriteReg(NRF24L01_ADDR_CONFIG, 0x00);
  HAL_Delay(10);
#ifdef NRF24L01_MASTER
  NRF24L01_WriteReg(NRF24L01_ADDR_CONFIG, NRF24L01_REG_CONFIG_TX);
#endif
#ifdef NRF24L01_SLAVE
  NRF24L01_WriteReg(NRF24L01_ADDR_CONFIG, NRF24L01_REG_CONFIG_RX);
#endif
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
  
#ifdef NRF24L01_MASTER
  // 主机CE初始低电平
  CE_LOW();
#endif
#ifdef NRF24L01_SLAVE
  // 从机CE保持高电平
  CE_HIGH();
#endif
}

void NRF24L01_Task(void)
{
  if(nrf24l01.rxFlag){
    nrf24l01.rxFlag = false;
    NRF24L01_Receive(nrf24l01.rxbuf, NRF24L01_DATA_SIZE);
    // 执行app操作
    // Modbus_Transmit(nrf24l01.rxbuf, NRF24L01_DATA_SIZE);
    // OLED_ShowDecNumber(2, 1, nrf24l01.rxbuf[0], 3);
    if (READ_BIT(nrf24l01.rxbuf[0], 1)) {
      LED_RED_TOGGLE();
    }
    if (READ_BIT(nrf24l01.rxbuf[0], 2)) {
      LED_GREEN_TOGGLE();
    }
    if (READ_BIT(nrf24l01.rxbuf[0], 4)) {
      LED_BLUE_TOGGLE();
    }
    // 处理完数据，清除FIFO
    flushRx();
  }
}

void NRF24L01_SetRxFlag(void)
{
  nrf24l01.rxFlag = true;
  clearRxIrq();
}