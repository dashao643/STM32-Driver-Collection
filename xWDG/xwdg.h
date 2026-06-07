#ifndef __XWDG_H
#define __XWDG_H

#include "iwdg.h"
#include "wwdg.h"
#include <stdint.h>

// 看门狗调试会复位

/********************* IWDG *********************/
// 喂狗最大间隔秒数计算公式：PSC * (Reload + 1) / 40000(LSI)
// 喂狗最大间隔：(256 * 4096) / 40000(LSI) = 26.2s
// 喂狗最小间隔：(4 * 1) / 40000(LSI) = 0.1ms

/********************* IWDG *********************/
void IWDG_Init(void);
void IWDG_Refresh(uint16_t intervalMs);

/********************* WWDG *********************/


/********************* WWDG *********************/

#endif
