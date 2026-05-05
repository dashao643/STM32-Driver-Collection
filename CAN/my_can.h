#ifndef __MY_CAN_H__
#define __MY_CAN_H__

#include "can.h"

HAL_StatusTypeDef CAN_SetFilter(void);
void CAN_TestPoll(uint16_t stdID, uint8_t *data, uint8_t size, uint8_t frameType);

#endif
