#ifndef __EC11_H__
#define __EC11_H__
#include "stm32f4xx.h"

void EC11_Init(void);
int16_t EC11_Get_Count(void);

#define BTN_SELECT  (1 << 0)
#define BTN_EXIT    (1 << 1)

void Button_Init(void);
uint8_t Button_Scan(void);
uint8_t Button_Get_State(void);

#endif // __EC11_H__

