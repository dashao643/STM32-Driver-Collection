#ifndef __XWDG_H
#define __XWDG_H

#include "iwdg.h"
#include "wwdg.h"
#include <stdint.h>

// 看门狗断点调试时也会复位

/********************* IWDG *********************/
// 喂狗间隔秒数计算公式：PSC * (Reload + 1) / 40KHz(LSI)
// 喂狗最大间隔：(256 * 4096) / 40000(LSI) = 26.2s
// 喂狗最小间隔：(4 * 1) / 40000(LSI) = 0.1ms
void IWDG_Init(void);
void IWDG_Refresh(uint16_t intervalMs);

/********************* WWDG *********************/
// 喂狗间隔秒数计算公式：4096 * PSC * (CNT - WIN + 1) / 36MHz(APB1)
// 喂狗最大间隔(36MHz)：(4096 * 8) * (127 - 64 + 1) / 36MHz = 58.25ms
// 喂狗最小间隔(36MHz)：(4096 * 1) * (x - x + 1) / 36MHz = 113us
void WWDG_Init(void);
void WWDG_Refresh(uint16_t intervalMs);

#endif
