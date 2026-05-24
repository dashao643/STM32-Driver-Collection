#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include "ds18b20.h"
#include "general.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "oled.h"
// DQ引脚配置为开漏输出，上拉，初始高电平

const uint8_t ROM1[DS18B20_ROM_SIZE] = {0x28, 0xF2, 0x0F, 0x02, 0x00, 0x02, 0x24, 0xB9};
uint8_t CONFIG[DS18B20_CONFIG_SIZE] = {DS18B20_CONFIG_TH, DS18B20_CONFIG_TL, DS18B20_CONFIG_12Bit};

static DS18B20_t ds18b20 = {0};

static inline void DQ_Set(GPIO_PinState pinState)
{
  HAL_GPIO_WritePin(DS18B20_DQ_GPIO_Port, DS18B20_DQ_Pin, pinState);
}

static inline GPIO_PinState DQ_Read(void)
{
  return HAL_GPIO_ReadPin(DS18B20_DQ_GPIO_Port, DS18B20_DQ_Pin);
}

/**
 * @brief 等待DQ总线变为低/高电平
 * 
 * @param pinState 变为低/高电平
 * @param timeoutUs 超时时间（单位：us）
 * @return true 等待成功
 * @return false 等待超时
 */
static bool DQ_Wait(GPIO_PinState pinState, uint16_t timeoutUs)
{
  uint16_t curTime = 0;

  do {
    if(DQ_Read() == pinState){
      return true;
    }
    curTime++;
    Delay_Us(1);
  } while(curTime < timeoutUs);

  return false;
}

void start(void)
{
  DQ_Set(PIN_LOW);
  Delay_Us(DS18B20_START_US);
  DQ_Set(PIN_HIGH);
  Delay_Us(DS18B20_WAIT_EXIST_US);
}

bool isExist(void)
{
  // 等待从机拉低
  if(!DQ_Wait(GPIO_PIN_RESET, DS18B20_EXIST_TIMEOUT_US)){
    return false;
  } 
  // 等待从机拉高
  return DQ_Wait(GPIO_PIN_SET, DS18B20_EXIST_TIMEOUT_US);
}

void writeBit(uint8_t bit)
{
  if((bit != 0) && (bit != 1)) {
    return;
  }
  DQ_Set(PIN_LOW);
  if(bit == 1){
    Delay_Us(DS18B20_WRITE_BIT_1_US);
  }
  else if(bit == 0){
    Delay_Us(DS18B20_WRITE_BIT_0_US);
  }
  DQ_Set(PIN_HIGH);

  // 等待足够的写入时长
  if(bit == 1){
    Delay_Us(DS18B20_WAIT_RW_BIT_US - DS18B20_WRITE_BIT_1_US);
  }
}

uint8_t readBit(void)
{
  DQ_Set(PIN_LOW);
  Delay_Us(DS18B20_READ_BIT_START_US);
  DQ_Set(PIN_HIGH);
  // 在持续时间的最后时间点读取
  Delay_Us(DS18B20_READ_BIT_SPAN_US - DS18B20_READ_BIT_START_US - 2);
  uint8_t bit = DQ_Read();
  Delay_Us(DS18B20_WAIT_RW_BIT_US - DS18B20_READ_BIT_SPAN_US);

  return bit;
}

// 先传低位
void writeByte(uint8_t byte)
{
  for(uint8_t i = 0; i < 8; i++){
    writeBit(byte & 0x01);
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

// void sendRom(uint8_t *rom)
// {
//   for(uint8_t i = 0; i < DS18B20_ROM_SIZE; i++){
//     writeByte(rom[i]);
//   }
// }

void sendCmdFun(uint8_t cmd, uint8_t func)
{
  start();
  if(!isExist()){
    printf("ds18b20 no response\n");
    return;
  }
  writeByte(cmd);
  writeByte(func);
}

void setConfig(void)
{
  sendCmdFun(DS18B20_SKIP_ROM, DS18B20_WRITE_SCRATCHPAD);

  // 10bit分辨率配置不上
  for(uint8_t i = 0; i < DS18B20_CONFIG_SIZE; i++){
    writeByte(CONFIG[i]);
  }
  // 读出9字节scratchpad
  Delay_Us(5);
  sendCmdFun(DS18B20_SKIP_ROM, DS18B20_READ_SCRATCHPAD);
  uint8_t scratchpad[DS18B20_SCRATCHPAD_SIZE];
  for(uint8_t i = 0; i < DS18B20_SCRATCHPAD_SIZE; i++){
    scratchpad[i] = readByte();
  }
  // 比对配置字节
  for(uint8_t i = 0; i < DS18B20_CONFIG_SIZE; i++){
    if(CONFIG[i] != scratchpad[i + 2]){
      printf("ds18b20 config error\n");
      return;
    }
  }
  // CRC校验
  uint8_t crc = CRC8_Maxim(scratchpad, DS18B20_SCRATCHPAD_SIZE - 1);
  if(crc != scratchpad[DS18B20_SCRATCHPAD_SIZE - 1]){
    printf("ds18b20 crc error\n");
    return;
  }
}

void updateTemp(void)
{
  sendCmdFun(DS18B20_SKIP_ROM, DS18B20_CONVERT_T);
  // 超时时间内等待读取到1
  uint8_t timeoutUs = 0;
  bool isTimeout = true;
  do {
    if(readBit() == 1){
      isTimeout = false;
      break;
    }
    timeoutUs++;
    Delay_Us(1);
  } while(timeoutUs < 100);

  if(isTimeout){
    printf("ds18b20 read timeout\n");
    return;
  }
  sendCmdFun(DS18B20_SKIP_ROM, DS18B20_READ_SCRATCHPAD);

  uint8_t data[2] = {0};
  for(uint8_t i = 0; i < 2; i++){
    data[i] = readByte();
  }
  printf("data0=%d ",data[0]);
  printf("data1=%d ",data[1]);

  int16_t rawTemp = (int16_t)((data[1] << 8) | data[0]);

  ds18b20.tempInt = rawTemp * 625 / 10000;
  ds18b20.tempDec = (rawTemp * 625 / 1000) % 100;  // 小数点后2位
  printf("ds18b20.tempInt=%d ",ds18b20.tempInt);
  printf("ds18b20.tempDec=%d ",ds18b20.tempDec);

  // 上电启动的五秒以内，不要读，读出来的是错的
}

/*-----------------------------------------------------------------*/

void DS18B20_Init(void)
{
  ds18b20.timer = HAL_GetTick();
  ds18b20.tempInt = 0;
  ds18b20.tempDec = 0;

  setConfig();
}

void DS18B20_Task(void)
{
  if((HAL_GetTick() - ds18b20.timer) < DS18B20_READ_INTERVAL_MS){
    return;
  }

  ds18b20.timer = HAL_GetTick();
  updateTemp();
  OLED_ShowDecNumber(4, 10, ds18b20.timer, 5);
}

void DS18B20_GetTemp(int8_t *tempInt, uint8_t *tempDec)
{
  *tempInt = ds18b20.tempInt;
  *tempDec = ds18b20.tempDec;
}
