#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include "ds18b20.h"
#include "general.h"

#include <stdint.h>
#include "modbus.h"
#include "oled.h"
#include "led.h"

/*
字节传输，先传低位
1.重置信号，检测从机存在
2.发命令
3.发函数
*/

/*
起始信号 拉低480us 释放
等待15us 读取是否低电平
写入1：拉低2us后释放
写入0：拉低60us-120us后释放
每一位操作中间间隔1us

读取时隙由主设备发起，将1-Wire总线拉低至少1微秒，然后释放总线
DS18B20通过保持总线高电平来传输1，通过将总线拉低来传输0
从DS18B20输出的数据在启动读取时隙的下降沿后的15微秒内有效
主机建议在接近15us的末尾读取
*/

// DQ引脚配置为开漏输出，上拉，初始高电平

static inline void DQ_Low(void)
{
  HAL_GPIO_WritePin(DS18B20_DQ_GPIO_Port, DS18B20_DQ_Pin, GPIO_PIN_RESET);
}

static inline void DQ_High(void)
{
  HAL_GPIO_WritePin(DS18B20_DQ_GPIO_Port, DS18B20_DQ_Pin, GPIO_PIN_SET);
}

static inline GPIO_PinState DQ_Read(void)
{
  return HAL_GPIO_ReadPin(DS18B20_DQ_GPIO_Port, DS18B20_DQ_Pin);
}

void start(void)
{
  DQ_Low();
  Delay_Us(DS18B20_START_US);
  DQ_High();
  Delay_Us(DS18B20_WAIT_EXIST_US);
}

bool exist(void)
{
  uint16_t timeout = 0;
  do {
    if(DQ_Read() == GPIO_PIN_RESET){
      // OLED_ShowDecNumber(2, 1, timeout, 4);
      return true;
    }
    timeout++;
    Delay_Us(1);
  } while(timeout <= DS18B20_EXIST_TIMEOUT_US);
  // OLED_ShowDecNumber(2, 1, timeout, 4);
  return false;
}

void writeBit(uint8_t bit)
{
  if((bit != 0) && (bit != 1)) {
    // LED_RED_TOGGLE();
    return;
  }
  // LED_GREEN_TOGGLE();
  // Modbus_Transmit(&bit, 1);
  DQ_Low();
  if(bit == 1){
    Delay_Us(DS18B20_WRITE_BIT_1_US);
  }
  else if(bit == 0){
    Delay_Us(DS18B20_WRITE_BIT_0_US);
  }
  DQ_High();

  // 等待足够的写入时长
  if(bit == 1){
    Delay_Us(DS18B20_WAIT_RW_BIT_US - DS18B20_WRITE_BIT_1_US);
  }
  Modbus_Transmit(&bit, 1);
}

uint8_t readBit(void)
{
  DQ_Low();
  Delay_Us(DS18B20_READ_BIT_START_US);
  DQ_High();
  // 在持续时间的最后时间点读取
  Delay_Us(DS18B20_READ_BIT_SPAN_US - DS18B20_READ_BIT_START_US - 4);
  uint8_t bit = DQ_Read();
  Delay_Us(DS18B20_WAIT_RW_BIT_US - DS18B20_READ_BIT_SPAN_US);

  return bit;
}

// 先传低位
void writeByte(uint8_t byte)
{
  for(uint8_t i = 0; i < 8; i++){
    writeBit(byte & 0x01);
    // uint8_t test = byte & 0x01;
    // Modbus_Transmit(&test, 1);
    byte >>= 1;
  }
}

// 先读低位
uint8_t readByte(void)
{
  uint8_t byte = 0;
  for(uint8_t i = 0; i < 8; i++){
    byte |= (readBit() << i);
  }

  return byte;
}

