#ifndef __DS1302_H__
#define __DS1302_H__

#include <stdint.h>
#include <stdbool.h>

#define OLED_SHOW_RTC

#define DS1302_CMD_WRITE              0x00
#define DS1302_CMD_READ               0x01

#define DS1302_CMD_SET_WP             0x80      // 设置写保护
#define DS1302_CMD_CLEAR_WP           0x00      // 清除写保护

#define DS1302_REG_MAX_SIZE           8         // 寄存器最大读写数量

#define DS1302_SECOND               0x80
#define DS1302_MINUTES              0x82
#define DS1302_HOUR                 0x84
#define DS1302_DATE                 0x86
#define DS1302_MONTH                0x88
#define DS1302_WEEK                 0x8A
#define DS1302_YEAR                 0x8C
#define DS1302_WP                   0x8E

#define DS1302_CLOCK_BURST          0xBE      // 连续读或写8字节

typedef struct {
  uint8_t year;         // 0-99
  uint8_t month;
  uint8_t date;
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
  uint8_t week;         // 1-7
}RTC_t;

void DS1302_ReadReg(uint8_t addr, uint8_t *data);
void DS1302_ReadRegs(uint8_t *data, uint8_t size);
void DS1302_WriteReg(uint8_t addr, uint8_t data);
void DS1302_WriteRegs(const uint8_t *data, uint8_t size);

void DS1302_Init(void);
void DS1302_GetRTC(RTC_t *rtc);
void DS1302_RTC_Show(void);

#endif
