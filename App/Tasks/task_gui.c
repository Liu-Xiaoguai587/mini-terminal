#include "FreeRTOS.h"
#include "task.h"
#include "gui_control.h"

void task_gui(void *pvParameters) {
    gui_control_init();
    gui_show_page();

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(5));
    }

    vTaskDelete(NULL);
}
