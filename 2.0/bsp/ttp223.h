#ifndef __TTP223_H__
#define __TTP223_H__

#include <stdbool.h>
#include <stdint.h>

void TTP223_Init(void);
void TTP223_EXTI_Callback(uint16_t GPIO_Pin);
bool TTP223_Task(void);

#endif
