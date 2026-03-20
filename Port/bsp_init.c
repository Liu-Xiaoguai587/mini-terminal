#include "ST7735s.h"
#include "EC11.h"
#include "stm32f4xx.h"


void bsp_init(void) {
    SystemInit(); 
    ST7735s_Init();
    EC11_Init();
    Button_Init();
}

