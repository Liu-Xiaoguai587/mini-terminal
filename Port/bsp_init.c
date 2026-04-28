#include "ST7735s.h"
#include "EC11.h"
#include "Sensors.h"
#include "I2C.h"
#include "USART2.h"
#include "stm32f4xx.h"


void bsp_init(void) {
    SystemInit();
    ST7735s_Init();
    IIC_Init();
    AHT10_Init();
    DS3231_Init();
    EC11_Init();
    Button_Init();
    USART2_Init(115200);
}

