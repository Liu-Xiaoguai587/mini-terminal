#include "FreeRTOS.h"
#include "task.h"

extern void gui_control_init(void);
extern void gui_show_page(void);

void task_gui(void *pvParameters) {
    gui_control_init();
        gui_show_page();

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    vTaskDelete(NULL);
}
