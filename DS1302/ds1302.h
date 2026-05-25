#ifndef __DS1302_H__
#define __DS1302_H__

#include <stdint.h>
#include <stdbool.h>

#define DS1302_CMD_WRITE        0x00
#define DS1302_CMD_READ         0x01


void DS1302_Init(void);

#endif
