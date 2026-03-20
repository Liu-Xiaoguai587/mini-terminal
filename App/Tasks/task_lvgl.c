#include "FreeRTOS.h"
#include "lvgl.h"
#include "task.h"
#include "gui_control.h"
#include "EC11.h"

void task_lvgl_base_timer(void *pvParameters) {
    gui_control_init();
    gui_show_page();

    uint8_t exit_last = 0;

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(5));

        Button_Scan();  // 更新 sel_stable / ext_stable

        uint8_t exit_now = (Button_Get_State() & BTN_EXIT) ? 1 : 0;
        if (exit_now && !exit_last) gui_pop_page();
        exit_last = exit_now;

        lv_timer_handler();
    }

    vTaskDelete(NULL);
}
