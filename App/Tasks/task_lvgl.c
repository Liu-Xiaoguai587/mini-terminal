#include "FreeRTOS.h"
#include "lvgl.h"
#include "task.h"

void task_lvgl_base_timer(void *pvParameters) {
    // lv_obj_t *lable = lv_label_create(lv_scr_act());
    // lv_label_set_text(lable, "Hello");
    // lv_obj_center(lable);

    for (;;) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(5));
    }


    vTaskDelete(NULL);
}





