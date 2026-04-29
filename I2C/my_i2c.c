#include "my_i2c.h"

#include "general.h"
#include "gpio.h"
#include "main.h"
#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdio.h>

// SDA 默认配置：开漏输出，高电平，上拉. 开漏输出可以读取外部信号的高低电平
// SCL 默认配置：推挽输出，高电平，上拉

#define SCL_HIGH()  HAL_GPIO_WritePin(I2C1_SCL_GPIO_Port, I2C1_SCL_Pin, GPIO_PIN_SET)
#define SCL_LOW()   HAL_GPIO_WritePin(I2C1_SCL_GPIO_Port, I2C1_SCL_Pin, GPIO_PIN_RESET)
#define SDA_HIGH()  HAL_GPIO_WritePin(I2C1_SDA_GPIO_Port, I2C1_SDA_Pin, GPIO_PIN_SET)
#define SDA_LOW()   HAL_GPIO_WritePin(I2C1_SDA_GPIO_Port, I2C1_SDA_Pin, GPIO_PIN_RESET)
#define SDA_READ()  HAL_GPIO_ReadPin(I2C1_SDA_GPIO_Port, I2C1_SDA_Pin)

static void start(void);
static void stop(void);
static void sendBit(uint8_t bit);
static uint8_t readBit(void);
static bool blockWaitAck(void);
static void sendAck(void);
static void sendNAck(void);
static void sendByte(uint8_t byte);
static uint8_t receiveByte(void);

// 数据通信过程中，每一次操作后，把SCL拉低
// SCL高的情况下SDA拉低
static void start(void)
{
  SDA_HIGH();
  SCL_HIGH();
  Delay_us(I2C_SOFTWARE_DELAY_US);
  SDA_LOW(); 
  Delay_us(I2C_SOFTWARE_DELAY_US);
  SCL_LOW();
}

// SCL高的情况下SDA拉高
static void stop(void)
{
  SDA_LOW(); 
  SCL_HIGH();
  Delay_us(I2C_SOFTWARE_DELAY_US);
  SDA_HIGH();
  Delay_us(I2C_SOFTWARE_DELAY_US);
}

// SDA先准备好数据,SDA默认高.拉高SCL发送数据
static void sendBit(uint8_t bit)
{
  bit &= 0x01;

  if(bit == 0)
    SDA_LOW();
  else
    SDA_HIGH();

  SCL_HIGH();
  Delay_us(I2C_SOFTWARE_DELAY_US);
  SCL_LOW();
  // SDA发完，释放SDA
  SDA_HIGH();
  Delay_us(I2C_SOFTWARE_DELAY_US);
}

static uint8_t readBit(void)
{
  SCL_LOW();
  Delay_us(I2C_SOFTWARE_DELAY_US);
  SCL_HIGH();
  Delay_us(I2C_SOFTWARE_DELAY_US);
  uint8_t bit = SDA_READ();
  SCL_LOW();

  return bit;
}

/**
 * @brief SCL拉高时检测SDA,检测到从机拉低表示有应答
 * 
 * @return true 有应答
 * @return false 无应答
 */
static bool blockWaitAck(void)
{
  uint16_t timeoutUs = 0;

  // 释放SDA，交给从机控制
  SDA_HIGH();
  SCL_HIGH();

  // 读取到高，继续循环阻塞，读取到低跳出while
  while(SDA_READ()){
    Delay_us(1);
    timeoutUs++;
    if(timeoutUs > I2C_SOFTWARE_TIMEOUT_US){
      // printf("timout=%d\n",timeoutUs);
      SCL_LOW();
      stop();
      return false;
    }
  }
  SCL_LOW();

  return true;
}

static void sendAck(void)
{
  SDA_LOW();
  SCL_HIGH();
  Delay_us(I2C_SOFTWARE_DELAY_US);
  SCL_LOW();
}

static void sendNAck(void)
{
  SDA_HIGH();
  SCL_HIGH();
  Delay_us(I2C_SOFTWARE_DELAY_US);
  SCL_LOW();
}

// 发送一字节数据，高位先行
static void sendByte(uint8_t byte)
{
  for(int8_t i = 0; i < 8; i++){
    sendBit(byte >> (8 - 1 - i));
  }
}

static uint8_t receiveByte(void)
{
  uint8_t byte = 0;
  for(int8_t i = 0; i < 8; i++){
    byte <<= 1;
    byte |= readBit();
  }
  return byte;
}


/**
 * @brief 写I2C从机内存
 * 
 * @param devAddress 从机写地址
 * @param memAddress 内存地址，最大16位
 * @param memAddSize 1字节或2字节
 * @param data 数据数组指针
 * @param size 数据字节大小
 * @return true 写成功
 * @return false 写失败
 */
bool I2C_Mem_Write(uint8_t devAddress, uint16_t memAddress, MemAddSize_e memAddSize, uint8_t *data, uint16_t size)
{
  start();

  // 发从机写地址,把最后一位置 0
  sendByte(devAddress & 0xFE);
  if(!blockWaitAck()){
    // printf("1\n");
    return false;
  }

  // 先发送高字节（如果是16位地址）
  if (memAddSize == I2C_MEMADD_16BIT) {
    sendByte((memAddress >> 8) & 0xFF);
    if (!blockWaitAck()) return false;
  }

  // 再发送低字节
  sendByte(memAddress & 0xFF);
  if (!blockWaitAck()) return false;

  // 循环发送数据
  for(uint16_t i = 0; i < size; i++){
    sendByte(data[i]);
    if(!blockWaitAck()){
      // printf("3\n");
      return false;
    }
  }

  stop();

  return true;
}

/**
 * @brief 读I2C从机内存
 * 
 * @param devAddress 从机读地址
 * @param memAddress 内存寻址数组指针
 * @param memAddSize 寻址字节大小
 * @param data 数据数组指针
 * @param size 数据字节大小
 * @return true 读成功
 * @return false 读失败
 */
bool I2C_Mem_Read(uint8_t devAddress, uint16_t memAddress, MemAddSize_e memAddSize, uint8_t *data, uint16_t size)
{
  start();

  // 发从机写地址,把最后一位置 0
  sendByte(devAddress & 0xFE);
  if(!blockWaitAck())
    return false;

  if (memAddSize == I2C_MEMADD_16BIT) {
    sendByte((memAddress >> 8) & 0xFF);
    if (!blockWaitAck()) return false;
  }

  sendByte(memAddress & 0xFF);
  if (!blockWaitAck()) return false;
  
  // 重复起始 + 发从机读地址,把最后一位置 1
  start();

  sendByte(devAddress | 0x01);
  if(!blockWaitAck())
    return false;

  for(uint16_t i = 0; i < size; i++){
    data[i] = receiveByte();
    if(i != size - 1)
      sendAck();    // 前面发 ACK
    else
      sendNAck();   // 最后一个字节发 NACK
  }
  stop();

  return true;
}