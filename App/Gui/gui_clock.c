#include "gui_page.h"
#include "sensor_data.h"

static lv_obj_t  *lbl_time;
static lv_obj_t  *lbl_date;
static lv_timer_t *poll_timer;

static void update_display(lv_timer_t *t) {
    (void)t;
    SensorData_t d = sensor_data_get();
    if (d.ds3231_ok) {
        lv_label_set_text_fmt(lbl_time, "%02d:%02d:%02d",
                              d.time.hour, d.time.min, d.time.sec);
        lv_label_set_text_fmt(lbl_date, "20%02d-%02d-%02d",
                              d.time.year, d.time.month, d.time.date);
    } else {
        lv_label_set_text(lbl_time, "--:--:--");
        lv_label_set_text(lbl_date, "----/--/--");
    }
}

static void clock_create(lv_obj_t *parent) {
    lbl_time = lv_label_create(parent);
    lv_label_set_text(lbl_time, "--:--:--");
    lv_obj_set_style_text_font(lbl_time, &lv_font_montserrat_14, 0);
    lv_obj_align(lbl_time, LV_ALIGN_CENTER, 0, -15);

    lbl_date = lv_label_create(parent);
    lv_label_set_text(lbl_date, "----/--/--");
    lv_obj_align(lbl_date, LV_ALIGN_CENTER, 0, 15);
}

static void clock_enter(void) {
    poll_timer = lv_timer_create(update_display, 500, NULL);
    update_display(NULL);
}

static void clock_leave(void) {
    if (poll_timer) {
        lv_timer_del(poll_timer);
        poll_timer = NULL;
    }
}

const PageDef_t page_clock = {
    .title      = "Clock",
    .on_create  = clock_create,
    .on_enter   = clock_enter,
    .on_leave   = clock_leave,
    .on_destroy = NULL,
    .sub_pages  = NULL,
    .sub_count  = 0,
};
