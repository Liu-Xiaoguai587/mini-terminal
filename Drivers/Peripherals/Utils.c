#include "stm32f4xx.h"
#include "lvgl.h"
#include "FreeRTOS.h"
#include "task.h"

volatile uint32_t g_rtos_fault_code;
volatile char     g_rtos_fault_task[configMAX_TASK_NAME_LEN];

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

static void store_task_name(const char *name)
{
    uint8_t i;

    if (!name) name = "?";
    for (i = 0; i < configMAX_TASK_NAME_LEN - 1 && name[i]; i++) {
        g_rtos_fault_task[i] = name[i];
    }
    g_rtos_fault_task[i] = '\0';
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    g_rtos_fault_code = 1;
    store_task_name(pcTaskName);
    taskDISABLE_INTERRUPTS();
    for (;;);
}

void vApplicationMallocFailedHook(void)
{
    TaskHandle_t cur = xTaskGetCurrentTaskHandle();

    g_rtos_fault_code = 2;
    store_task_name(pcTaskGetName(cur));
    taskDISABLE_INTERRUPTS();
    for (;;);
}

