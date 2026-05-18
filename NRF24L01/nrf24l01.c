#include "nrf24l01_master.h"
#include "general.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_def.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "gpio.h"
/* 
默认配置：
TX,RX 5Byte地址
自动重传间隔250us,3次
通信速率2Mbps
TX默认发往管道0
定长数据字节数
*/

/*
将配置位PRIM_RX设置为低电平。
当应用MCU有数据要传输时，通过SPI将接收节点的地址（TX_ADDR）和有效载荷数据（TX_PLD）加载到nRF24L01+中。
TX有效载荷的宽度从MCU写入TX FIFO的字节数开始计算。在保持CSN低电平的同时，必须连续写入TX_PLD。如果TX_ADDR与上次传输时相同，则无需重写。
如果PTX设备需要接收确认，则配置数据管道0以接收ACK包。数据管道0的RX地址（RX_ADDR_P0）必须与PTX设备中的TX地址（TX_ADDR）相同。
以第41页图14中的示例为例，TX5设备和RX设备的地址设置如下：TX5设备：TX_ADDR = 0xB3B4B5B605 TX5设备：RX_ADDR_P0 = 0xB3B4B5B605 
RX设备：RX_ADDR_P5 = 0xB3B4B5B605 
CE上的高电平脉冲启动传输。CE上的最小脉冲宽度为10µs。

数据以高速（由MCU配置的1Mbps或2Mbps）传输。如果激活了自动确认（ENAA_P0=1），则无线电立即进入接收（RX）模式，
除非在接收到的数据包中设置了NO_ACK位。如果在有效确认时间窗口内接收到有效数据包，则传输视为成功。状态寄存器中的TX_DS位设置为高，
有效载荷从发送FIFO（TX FIFO）中移除。如果在指定的时间窗口内未接收到有效的确认数据包，则重新传输有效载荷（如果启用了自动重传）。
如果自动重传计数器（ARC_CNT）超过编程设定的最大限制（ARC），则状态寄存器中的MAX_RT位设置为高。发送FIFO中的有效载荷不会被移除。
当MAX_RT或TX_DS为高时，IRQ引脚处于活动状态。要关闭IRQ引脚，请通过写入状态寄存器来重置中断源（请参阅中断章节）。
如果在最大重传次数后仍未接收到针对某个数据包的确认数据包，则在MAX_RT中断清除之前，无法进一步传输数据包。
每次MAX_RT中断时，数据包丢失计数器（PLOS_CNT）都会增加。即，ARC_CNT统计的是为传输单个数据包所需的重新传输次数。
PLOS_CNT统计的是在最大重传次数后仍未传输成功的数据包数量。

如果CE（使能）为低，nRF24L01+将进入待机-I模式。否则，将发送TX FIFO（发送FIFO）中的下一个有效载荷。如果TX FIFO为空且CE仍然为高，
nRF24L01+将进入待机-II模式。
如果nRF24L01+处于待机-II模式，一旦CE置为低电平，它就会立即切换到待机-I模式
*/

/*
发送：PWR_UP 拉高 PRIM_RX 拉低 FIFO发数据 拉高CE大于10us ，发完回到待机2，拉低CE回到待机1。
待机2模式下，FIFO发数据 拉高CE，自动发送。
静态有效载荷长度，发送端由TX_FIFO时钟的字节数决定，由接收端的RX_PW_Px寄存器决定
发送完成以后，若收到ACK，会置TX_DS中断，IRQ默认配置为高，发送完成以后会拉低
接收模式CE要持续拉高

*/
static uint32_t timer = 0;


static inline void CS_LOW(void);
static inline void CS_HIGH(void);
static inline void CE_LOW(void);
static inline void CE_HIGH(void);
static inline HAL_StatusTypeDef transmit(const uint8_t *data, uint16_t size);
static inline HAL_StatusTypeDef receive(uint8_t *data, uint16_t size);
static void cmdTransmit(uint8_t cmd);
// static bool isBusy(void);
// static bool waitBusyTimeout(void);
// static bool isNeedRMW(uint16_t page, uint16_t addrInPage, uint16_t size);
// static HAL_StatusTypeDef writeDirectly(uint32_t addr, const uint8_t *data, uint16_t size);

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

	// 底层寄存器写入（独立CS周期）
static inline void writeReg(uint8_t regAddr, uint8_t data)
{
  if(regAddr > 0x1F) return;
  regAddr |= 0x20;
  CS_LOW();
  transmit(&regAddr, 1);
  transmit(&data, 1);
  CS_HIGH();
}

// 底层寄存器读取（独立CS周期）
static inline void readReg(uint8_t regAddr, uint8_t* data)
{
  if(regAddr > 0x1F) return;
  CS_LOW();
  transmit(&regAddr, 1);
  receive(data, 1);
  CS_HIGH();
}

// static inline void writeReg(uint8_t regAddr, const uint8_t *data, uint8_t size)
// {
//   if(size == 0 || size > 5) return;
//   if(regAddr > 0x1F) return;
//   regAddr |= 0x20;
//   transmit(&regAddr, 1);
//   transmit(data, size);
// }

	// 底层纯命令发送（独立CS周期）
static void cmdTransmit(uint8_t cmd)
{
  CS_LOW();
  transmit(&cmd, 1);
  CS_HIGH();
}



void NRF24L01_Init(void)
{
	writeReg(NRF24L01_REG_ADDR_CONFIG, NRF24L01_REG_POWER_PTX);
  CE_HIGH();
}

HAL_StatusTypeDef NRF24L01_Transmit(uint8_t data)
{
  // 此包无ack
  cmdTransmit(NRF24L01_CMD_NO_ACK);

  CS_LOW();
  uint8_t cmd = NRF24L01_CMD_W_TX_PAYLOAD;
  transmit(&cmd, 1);
  // 紧接着发送数据
  HAL_StatusTypeDef state = transmit(&data, NRF24L01_DATA_SIZE);
  CS_HIGH(); // 拉高CS，完成一次完整的事务，NRF24L01将数据载入FIFO

  printf("state=%d\n",state);
  if(state != HAL_OK) return state;
  // cmdTransmit(NRF24L01_CMD_FLUSH_TX);

  CE_HIGH();
  Delay_us(20);
  // CE_LOW();

  // timer = HAL_GetTick();
  // 发送完成后 等待中断收到ack置标志
  // while(){
  //   if((HAL_GetTick() - timer) > NRF24L01_WAIT_ACK_MS))
  //     return HAL_TIMEOUT;
  // }
  return HAL_OK;
}

bool NRF24L01_WaitAck(void)
{

  return true;
}

void NRF24L01_ReadReg(uint8_t *data)
{
  CS_LOW();
  uint8_t cmd = 0x00;  // 读CONFIG寄存器
  transmit(&cmd, 1);
  receive(data, 1);
  CS_HIGH();
}