#include "stm32f4xx.h"
#include "lvgl.h"
#include "FreeRTOS.h"


// void SysTick_Configuration(void) {
//     if (SysTick_Config(SystemCoreClock / 1000)) {
//         // Capture error
//         while (1);
//     }
// }

volatile uint32_t g_TimingDelay;
void vApplicationTickHook(void) {
    lv_tick_inc(1);
    
    if (g_TimingDelay != 0)
    {
        g_TimingDelay--;
    }

}
void Delay_ms(uint32_t nTime)
{
    g_TimingDelay = nTime;

    while(g_TimingDelay != 0);
}

