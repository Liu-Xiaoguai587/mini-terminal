#include "FreeRTOS.h"
#include "gui_page.h"
#include "sensor_data.h"

extern TaskHandle_t task_sensor_handle;

static lv_obj_t  *lbl_temp;
static lv_obj_t  *lbl_hum;
static lv_timer_t *poll_timer;

static void update_display(lv_timer_t *t) {
    (void)t;
    SensorData_t d = sensor_data_get();
    if (d.aht10_ok) {
        int t_int = (int)d.temperature, t_dec = ((int)(d.temperature * 10)) % 10;
        int h_int = (int)d.humidity,    h_dec = ((int)(d.humidity * 10)) % 10;
        if (t_dec < 0) t_dec = -t_dec;
        if (h_dec < 0) h_dec = -h_dec;
        lv_label_set_text_fmt(lbl_temp, "Temp: %d.%d C", t_int, t_dec);
        lv_label_set_text_fmt(lbl_hum,  "Hum:  %d.%d %%", h_int, h_dec);
    } else {
        lv_label_set_text(lbl_temp, "Temp: --");
        lv_label_set_text(lbl_hum,  "Hum:  --");
    }
}

static void sensor_create(lv_obj_t *parent) {
    lbl_temp = lv_label_create(parent);
    lv_label_set_text(lbl_temp, "Temp: --");
    lv_obj_align(lbl_temp, LV_ALIGN_TOP_MID, 0, 20);

    lbl_hum = lv_label_create(parent);
    lv_label_set_text(lbl_hum, "Hum:  --");
    lv_obj_align(lbl_hum, LV_ALIGN_TOP_MID, 0, 50);
}

static void sensor_enter(void) {
    poll_timer = lv_timer_create(update_display, 500, NULL);
    update_display(NULL);

    eTaskState state = eTaskStateGet(task_sensor_handle);
    if (state == eSuspended) {
        vTaskResume(task_sensor_handle);
    } 
}

static void sensor_leave(void) {
    if (poll_timer) {
        lv_timer_del(poll_timer);
        poll_timer = NULL;
    }

    eTaskState state = eTaskStateGet(task_sensor_handle);
    if (state != eSuspended) {
        vTaskSuspend(task_sensor_handle);
    } 
}

const PageDef_t page_sensor = {
    .title      = "Sensors",
    .on_create  = sensor_create,
    .on_enter   = sensor_enter,
    .on_leave   = sensor_leave,
    .on_destroy = NULL,
    .sub_pages  = NULL,
    .sub_count  = 0,
};
