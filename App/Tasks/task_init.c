#include "FreeRTOS.h"
#include "task.h"

extern void task_lvgl_base_timer(void *pvParameters);
extern void task_gui(void *pvParameters);

void task_init(void) {
    // init lvgl timer base
    xTaskCreate(
        task_lvgl_base_timer,
        "lvgl_base_timer",
        2048,
        NULL,
        3,
        NULL
    );

    xTaskCreate(
        task_gui,
        "task_gui",
        2048,
        NULL,
        2,
        NULL
    );

}
