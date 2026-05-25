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

static inline void DQ_Set(GPIO_PinState pinState);
static inline GPIO_PinState DQ_Read(void);
static bool DQ_Wait(GPIO_PinState pinState, uint16_t timeoutUs);
static void start(void);
static bool isExist(void);
static void writeBit(uint8_t bit);
static uint8_t readBit(void);
static void writeByte(uint8_t byte);
static uint8_t readByte(void);
static bool crcCheck(const uint8_t scratchpad[], uint8_t size);
static void sendRom(uint8_t *rom) UNUSED_FUNC;
static void sendCmdFun(uint8_t cmd, uint8_t func);
static void setConfig(void);
static void updateTemp(void);

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

static void start(void)
{
  DQ_Set(PIN_LOW);
  Delay_Us(DS18B20_START_US);
  DQ_Set(PIN_HIGH);
  Delay_Us(DS18B20_WAIT_EXIST_US);
}

static bool isExist(void)
{
  // 等待从机拉低
  if(!DQ_Wait(GPIO_PIN_RESET, DS18B20_EXIST_TIMEOUT_US)){
    return false;
  } 
  // 等待从机拉高
  return DQ_Wait(GPIO_PIN_SET, DS18B20_EXIST_TIMEOUT_US);
}

static void writeBit(uint8_t bit)
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

static uint8_t readBit(void)
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
static void writeByte(uint8_t byte)
{
  for(uint8_t i = 0; i < 8; i++){
    writeBit(byte & 0x01);
    byte >>= 1;
  }
}

// 先读低位
static uint8_t readByte(void)
{
  uint8_t byte = 0;
  for(uint8_t i = 0; i < 8; i++){
    byte |= (readBit() << i);
  }

  return byte;
}

static bool crcCheck(const uint8_t scratchpad[], uint8_t size)
{
  if(size != DS18B20_SCRATCHPAD_SIZE) return false;

  uint8_t crcCal = CRC8_Maxim(scratchpad, DS18B20_SCRATCHPAD_SIZE - 1);
  if(crcCal != scratchpad[DS18B20_SCRATCHPAD_SIZE - 1]){
    printf("ds18b20 crc error\n");
    return false;
  }
  return true;
}

static void sendRom(uint8_t *rom)
{
  for(uint8_t i = 0; i < DS18B20_ROM_SIZE; i++){
    writeByte(rom[i]);
  }
}

static void sendCmdFun(uint8_t cmd, uint8_t func)
{
  start();
  if(!isExist()){
    printf("ds18b20 no response\n");
    return;
  }
  writeByte(cmd);
  writeByte(func);
}

static void setConfig(void)
{
  // 配置命令（10bit分辨率配置不上）
  sendCmdFun(DS18B20_SKIP_ROM, DS18B20_WRITE_SCRATCHPAD);
  for(uint8_t i = 0; i < DS18B20_CONFIG_SIZE; i++){
    writeByte(CONFIG[i]);
  }

  // 配置过后读出验证
  Delay_Us(5);
  sendCmdFun(DS18B20_SKIP_ROM, DS18B20_READ_SCRATCHPAD);

  uint8_t scratchpad[DS18B20_SCRATCHPAD_SIZE];

  for(uint8_t i = 0; i < DS18B20_SCRATCHPAD_SIZE; i++){
    scratchpad[i] = readByte();
  }
  for(uint8_t i = 0; i < DS18B20_CONFIG_SIZE; i++){
    if(CONFIG[i] != scratchpad[i + 2]){
      printf("ds18b20 config error\n");
      return;
    }
  }

  // CRC校验
  if(!crcCheck(scratchpad, DS18B20_SCRATCHPAD_SIZE)){
    return;
  }

  // 存储到EE
  Delay_Us(5);
  sendCmdFun(DS18B20_SKIP_ROM, DS18B20_COPY_SCRATCHPAD);

  // 初始转换温度一次
  Delay_Us(5);
  sendCmdFun(DS18B20_SKIP_ROM, DS18B20_CONVERT_T);
}

static void updateTemp(void)
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

  uint8_t scratchpad[DS18B20_SCRATCHPAD_SIZE] = {0};
  for(uint8_t i = 0; i < DS18B20_SCRATCHPAD_SIZE; i++){
    scratchpad[i] = readByte();
  }
  if(!crcCheck(scratchpad, DS18B20_SCRATCHPAD_SIZE)){
    return;
  }
	
  int16_t rawTemp = (int16_t)((scratchpad[1] << 8) | scratchpad[0]);

  ds18b20.tempInt = rawTemp * 625 / 10000;
  ds18b20.tempDec = (rawTemp * 625 / 100) % 100;  // 小数点后2位
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

  // 处理成字符串显示:XX.XX°C
  char tempStr[7] = {0};
  int16_t temp4Digit = ds18b20.tempInt * 100 + ds18b20.tempDec;
  // printf("temp4Digit=%d\n",temp4Digit);
  IntToString_2(temp4Digit, tempStr, 7);
  OLED_ShowString(2, 1, tempStr);
}

void DS18B20_GetTemp(int8_t *tempInt, uint8_t *tempDec)
{
  *tempInt = ds18b20.tempInt;
  *tempDec = ds18b20.tempDec;
}
