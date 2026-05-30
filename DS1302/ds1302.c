#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include "ds1302.h"
#include "general.h"

#include <stdint.h>
#include <stdio.h>

#ifdef OLED_SHOW_RTC
#include "oled.h"
#endif

static inline void CE_Set(GPIO_PinState pinState);
static inline void CLK_Set(GPIO_PinState pinState);
static inline void DAT_SetPin(GPIO_PinState pinState);
static void DAT_SetMode(uint32_t mode);
static void writeBit(uint8_t bit, bool isRead);
static void writeByte(uint8_t byte, bool isRead);
static uint8_t readBit(void);
static uint8_t readByte(void);
static void cmdSend(uint8_t rw, uint8_t cmd);
static void setWP(void);
static void clearWP(void);

// PIN_HIGH / PIN_LOW
static inline void CE_Set(GPIO_PinState pinState)
{
  HAL_GPIO_WritePin(DS1302_CE_GPIO_Port , DS1302_CE_Pin, pinState);
}

static inline void CLK_Set(GPIO_PinState pinState)
{
  HAL_GPIO_WritePin(DS1302_CLK_GPIO_Port , DS1302_CLK_Pin, pinState);
}

static inline void DAT_SetPin(GPIO_PinState pinState)
{
  HAL_GPIO_WritePin(DS1302_DAT_GPIO_Port , DS1302_DAT_Pin, pinState);
}

// GPIO_MODE_INPUT / GPIO_MODE_OUTPUT_PP
static void DAT_SetMode(uint32_t mode)
{
  if(mode != GPIO_MODE_INPUT && mode != GPIO_MODE_OUTPUT_PP) return;

  GPIO_InitTypeDef gpio;
  gpio.Mode = mode;
  gpio.Pin = DS1302_DAT_Pin;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_MEDIUM;

  HAL_GPIO_Init(DS1302_DAT_GPIO_Port, &gpio);
  DAT_SetPin(PIN_LOW);
}

// 内部函数不操作CE,如果下一时钟是读操作，CLK不拉低
static void writeBit(uint8_t bit, bool isRead)
{
  if(bit != 0 && bit != 1) return;

  DAT_SetPin((GPIO_PinState)bit);
  CLK_Set(PIN_HIGH);
  Delay_Us(1);
  if(!isRead){
    CLK_Set(PIN_LOW);
    Delay_Us(1);
  }
}

// 先传低位
static void writeByte(uint8_t byte, bool isRead)
{
  DAT_SetMode(GPIO_MODE_OUTPUT_PP);
  for(uint8_t i = 0; i < 8; i++){
    // 是读操作并且在最后一个字节，CLK不拉低
    writeBit(byte & 0x01, isRead && (i == 7));
    byte >>= 1;
  }
}

static uint8_t readBit(void)
{
  CLK_Set(PIN_LOW);
  uint8_t bit = HAL_GPIO_ReadPin(DS1302_DAT_GPIO_Port, DS1302_DAT_Pin);
  Delay_Us(1);
  CLK_Set(PIN_HIGH);
  Delay_Us(1);

  return bit;
}

static uint8_t readByte(void)
{
  DAT_SetMode(GPIO_MODE_INPUT);
  uint8_t byte = 0;
  for(uint8_t i = 0; i < 8; i++){
    byte |= (readBit() << i);
  }
  CLK_Set(PIN_LOW);

  return byte;
}

// DS1302_CMD_WRITE / DS1302_CMD_READ
static void cmdSend(uint8_t rw, uint8_t cmd)
{
  if(rw != DS1302_CMD_WRITE && rw != DS1302_CMD_READ) return;

  writeByte(rw | cmd, (bool)rw);
}

static void setWP(void)
{
  CE_Set(PIN_HIGH);
  cmdSend(DS1302_CMD_WRITE, DS1302_WP);
  writeByte(DS1302_CMD_SET_WP, false);
  CE_Set(PIN_LOW);
}

static void clearWP(void)
{
  CE_Set(PIN_HIGH);
  cmdSend(DS1302_CMD_WRITE, DS1302_WP);
  writeByte(DS1302_CMD_CLEAR_WP, false);
  CE_Set(PIN_LOW);
}

/*-----------------------------------------------------------------*/

/**
 * @brief 读DS1302单个寄存器
 * 
 * @param addr 寄存器地址
 * @param data 字节数据
 */
void DS1302_ReadReg(uint8_t addr, uint8_t *data)
{
  CE_Set(PIN_HIGH);
  cmdSend(DS1302_CMD_READ, addr);
  *data = readByte();
  CE_Set(PIN_LOW);
}

/**
 * @brief 连续读DS1302寄存器，最多读8个
 * 
 * @param data 数据字节
 * @param size 字节大小
 */
void DS1302_ReadRegs(uint8_t *data, uint8_t size)
{
  if (size > DS1302_REG_MAX_SIZE) size = DS1302_REG_MAX_SIZE;

  // 连续读
  CE_Set(PIN_HIGH);
  cmdSend(DS1302_CMD_READ, DS1302_CLOCK_BURST);
  for(uint8_t i = 0; i < size; i++){
    data[i] = readByte();
  }
  CE_Set(PIN_LOW);
}

/**
 * @brief 写单个寄存器
 * 
 * @param addr 寄存器地址
 * @param data 1字节数据
 */
void DS1302_WriteReg(uint8_t addr, uint8_t data)
{
  // 清除写保护
  clearWP();

  // 写入数据
  CE_Set(PIN_HIGH);
  cmdSend(DS1302_CMD_WRITE, addr);
  writeByte(data, false);
  CE_Set(PIN_LOW);

  // 恢复写保护
  setWP();
}

/**
 * @brief 连续写最多8个寄存器 (秒 分 时 日 月 周 年 WP)
 * 
 * @param data 字节数据
 * @param size 字节大小
 */
void DS1302_WriteRegs(const uint8_t *data, uint8_t size)
{
  if(size > DS1302_REG_MAX_SIZE) size = DS1302_REG_MAX_SIZE;

  clearWP();

  CE_Set(PIN_HIGH);
  cmdSend(DS1302_CMD_WRITE, DS1302_CLOCK_BURST);
  for(uint8_t i = 0; i < size; i++){
    writeByte(data[i], false);
  }
  CE_Set(PIN_LOW);

  setWP();
}

void DS1302_Init(void)
{
  DAT_SetMode(GPIO_MODE_OUTPUT_PP);
  CE_Set(PIN_LOW);
  CLK_Set(PIN_LOW);
  DAT_SetPin(PIN_LOW);
  // 开启振荡器
  uint8_t sec = 0;
  DS1302_ReadReg(DS1302_SECOND, &sec);
  if (sec & 0x80) {
    DS1302_WriteReg(DS1302_SECOND, sec & 0x7F);
  }
}

/**
 * @brief 获取rtc结构体
 * 
 * @param rtc RTC_t结构体
 */
void DS1302_GetRTC(RTC_t *rtc)
{
  uint8_t rtcArr[7] = {0};

  DS1302_ReadRegs(rtcArr, 7);
  // BCD转10进制
  for(uint8_t i = 0; i < 7; i++){
    rtcArr[i] = (rtcArr[i] >> 4) * 10 + (rtcArr[i] & 0x0F);
  }
  // 秒 分 时 日 月 周 年
  rtc->seconds = rtcArr[0];
  rtc->minutes = rtcArr[1];
  rtc->hours = rtcArr[2];
  rtc->date = rtcArr[3];
  rtc->month = rtcArr[4];
  rtc->week = rtcArr[5];
  rtc->year = rtcArr[6];
}

void DS1302_RTC_Show(void)
{
  RTC_t rtc;
  char dateBuf[17] = {0};
  char timeBuf[17] = {0};

  DS1302_GetRTC(&rtc);
  
  snprintf(dateBuf, sizeof(dateBuf), "%04d-%02d-%02d:%d", 2000 + rtc.year, rtc.month, rtc.date, rtc.week);
  snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d", rtc.hours, rtc.minutes, rtc.seconds);
#ifdef OLED_SHOW_RTC
  OLED_ShowString(3, 1, dateBuf);
  OLED_ShowString(4, 1, timeBuf);
#endif
}
