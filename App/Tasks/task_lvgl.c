#include "FreeRTOS.h"
#include "task.h"
#include "lvgl.h"
#include "gui_page.h"
#include "EC11.h"

extern const PageDef_t page_main_menu;

void task_lvgl(void *pvParameters) {
    page_mgr_init(&page_main_menu);

    uint8_t exit_last = 0;

    for (;;) {
        Button_Scan();

        /* EXIT button rising-edge → pop page */
        uint8_t exit_now = (Button_Get_State() & BTN_EXIT) ? 1 : 0;
        if (exit_now && !exit_last) page_mgr_pop();
        exit_last = exit_now;

        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
