#ifndef __EC11_H__
#define __EC11_H__
#include "stm32f4xx.h"

void EC11_Init(void);
int16_t EC11_Get_Count(void);
uint8_t EC11_Get_SW_State(void);

#endif // __EC11_H__

