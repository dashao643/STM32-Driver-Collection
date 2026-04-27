#include "my_i2c.h"

#include "general.h"
#include "main.h"
#include "stm32f1xx_hal.h"
#include <stdint.h>

// SDA 默认配置：开漏输出，高电平，上拉. 开漏输出可以读取外部信号的高低电平
// SCL 默认配置：推挽输出，高电平，上拉

#define SCL_HIGH()  HAL_GPIO_WritePin(I2C1_SCL_GPIO_Port, I2C1_SCL_Pin, GPIO_PIN_SET)
#define SCL_LOW()   HAL_GPIO_WritePin(I2C1_SCL_GPIO_Port, I2C1_SCL_Pin, GPIO_PIN_RESET)
#define SDA_HIGH()  HAL_GPIO_WritePin(I2C1_SDA_GPIO_Port, I2C1_SDA_Pin, GPIO_PIN_SET)
#define SDA_LOW()   HAL_GPIO_WritePin(I2C1_SDA_GPIO_Port, I2C1_SDA_Pin, GPIO_PIN_RESET)
#define SDA_READ()  HAL_GPIO_ReadPin(I2C1_SDA_GPIO_Port, I2C1_SDA_Pin)

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
  uint8_t timeoutMs = 0;

  SCL_HIGH();

  // 读取到高，继续循环阻塞，读取到低跳出while
  while(SDA_READ()){
    HAL_Delay(1);
    timeoutMs++;
    if(timeoutMs > I2C_SOFTWARE_TIMEOUT_MS){
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

bool I2C_Transmit(uint8_t devAddress, uint8_t *data, uint16_t size)
{
  start();

  // 先发从机地址
  sendByte(devAddress);
  if(!blockWaitAck())
    return false;

  // 循环发送数据
  for(uint16_t i = 0; i < size; i++){
    sendByte(data[i]);
    if(!blockWaitAck())
      return false;
  }

  stop();

  return true;
}

bool I2C_Receive(uint8_t devAddress, uint8_t *data, uint16_t size)
{
  start();

  // 先发从机地址
  sendByte(devAddress);
  if(!blockWaitAck())
    return false;

  // 循环接收数据
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

bool I2C_Mem_Write(uint8_t devAddress, uint8_t memAddress, uint8_t memAddSize, uint8_t *data, uint16_t size)
{
  start();

  // 发从机地址
  sendByte(devAddress);
  if(!blockWaitAck())
    return false;

  // 发寄存器地址
  for(uint8_t i = 0; i < memAddSize; i++){
    sendByte(memAddress);
    if(!blockWaitAck())
    return false;
  }
  
  // 循环发送数据
  for(uint16_t i = 0; i < size; i++){
    sendByte(data[i]);
    if(!blockWaitAck())
      return false;
  }
  stop();

  return true;
}

bool I2C_Mem_Read(uint8_t devAddress, uint8_t memAddress, uint8_t memAddSize, uint8_t *data, uint16_t size)
{
  start();

  // 发从机地址
  sendByte(devAddress);
  if(!blockWaitAck())
    return false;

  // 发寄存器地址
  sendByte(memAddress);
  if(!blockWaitAck())
    return false;
  
  // 重复起始 + 读地址
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