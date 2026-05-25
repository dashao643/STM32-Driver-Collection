#ifndef __DS1302_DEF_H__
#define __DS1302_DEF_H__

#define DS1302_SECOND               0x80
#define DS1302_MINUTES              0x82
#define DS1302_HOUR                 0x84
#define DS1302_DATE                 0x86
#define DS1302_MONTH                0x88
#define DS1302_WEEK                 0x8A
#define DS1302_YEAR                 0x8C
#define DS1302_WP                   0x8E

#define DS1302_CLOCK_BURST          0xBE      // 连续写入8字节

#endif
