#include "FreeRTOS.h"
#include "task.h"

extern void task_lvgl_base_timer(void *pvParameters);

void task_init(void) {
    xTaskCreate(
        task_lvgl_base_timer,
        "lvgl_base_timer",
        2048,
        NULL,
        3,
        NULL
    );
}
