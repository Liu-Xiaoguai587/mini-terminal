#include "gui_page.h"
#include "gui_theme.h"
#include "sensor_data.h"

static lv_obj_t   *s_time_label;
static lv_obj_t   *s_date_label;
static lv_obj_t   *s_link_label;
static lv_obj_t   *s_mem_label;
static lv_timer_t *s_home_timer;

static void home_update_labels(lv_timer_t *timer) {
    SensorData_t d;

    (void)timer;
    if (!s_time_label || !s_date_label) return;

    d = sensor_data_get();
    if (d.ds3231_ok) {
        lv_label_set_text_fmt(s_time_label, "%02u:%02u",
                              (unsigned)d.time.hour,
                              (unsigned)d.time.min);
        lv_label_set_text_fmt(s_date_label, "%02u.%02u.20%02u",
                              (unsigned)d.time.month,
                              (unsigned)d.time.date,
                              (unsigned)d.time.year);
        lv_label_set_text(s_link_label, " RTC[OK] ");
    } else {
        lv_label_set_text(s_time_label, "--:--");
        lv_label_set_text(s_date_label, "--.--.----");
        lv_label_set_text(s_link_label, " RTC[--] ");
    }

    if (d.aht10_ok) {
        lv_label_set_text(s_mem_label, " AHT[OK] ");
    } else {
        lv_label_set_text(s_mem_label, " AHT[--] ");
    }
}

void gui_home_render(lv_obj_t *parent) {
    lv_obj_t *time;
    lv_obj_t *date;
    lv_obj_t *line;
    lv_obj_t *link;
    lv_obj_t *mem;

    gui_plain_obj(parent, lv_color_hex(GUI_BLACK));
    lv_obj_set_style_pad_all(parent, 0, 0);

    time = gui_label(parent, "--:--", lv_color_hex(GUI_AMBER));
    s_time_label = time;
    lv_obj_set_style_text_font(time, GUI_FONT_HUGE, 0);
    lv_obj_set_style_text_align(time, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(time, LV_ALIGN_TOP_MID, 0, 0);

    line = lv_obj_create(parent);
    lv_obj_set_size(line, 86, 1);
    gui_plain_obj(line, lv_color_hex(GUI_AMBER));
    lv_obj_set_style_bg_opa(line, LV_OPA_70, 0);
    lv_obj_align(line, LV_ALIGN_TOP_MID, 0, 36);

    date = gui_label(parent, "--.--.----", lv_color_hex(GUI_AMBER));
    s_date_label = date;
    lv_obj_set_style_text_font(date, GUI_FONT_SMALL, 0);
    lv_obj_set_style_text_opa(date, LV_OPA_80, 0);
    lv_obj_align(date, LV_ALIGN_TOP_MID, 0, 39);

    link = gui_label(parent, " RTC[--] ", lv_color_hex(GUI_BLACK));
    s_link_label = link;
    lv_obj_set_style_text_font(link, GUI_FONT_SMALL, 0);
    gui_clip_label(link, 56);
    lv_obj_set_style_bg_color(link, lv_color_hex(GUI_AMBER), 0);
    lv_obj_set_style_bg_opa(link, LV_OPA_COVER, 0);
    lv_obj_align(link, LV_ALIGN_BOTTOM_LEFT, 6, -12);

    mem = gui_label(parent, " AHT[--] ", lv_color_hex(GUI_AMBER));
    s_mem_label = mem;
    lv_obj_set_style_text_font(mem, GUI_FONT_SMALL, 0);
    gui_clip_label(mem, 56);
    lv_obj_set_style_border_width(mem, 1, 0);
    lv_obj_set_style_border_color(mem, lv_color_hex(GUI_AMBER), 0);
    lv_obj_align(mem, LV_ALIGN_BOTTOM_RIGHT, -6, -12);

    gui_status_cursor(parent, ">");
    home_update_labels(NULL);
}

static void home_create(lv_obj_t *parent) {
    lv_obj_t *content = gui_page_content(parent, "[SYSTEM_READY]");
    gui_home_render(content);
}

static void home_enter(void) {
    if (!s_home_timer) {
        s_home_timer = lv_timer_create(home_update_labels, 1000, NULL);
    }
    home_update_labels(NULL);
}

static void home_leave(void) {
    if (s_home_timer) {
        lv_timer_del(s_home_timer);
        s_home_timer = NULL;
    }
}

const PageDef_t page_home = {
    .title      = "[SYSTEM_READY]",
    .on_create  = home_create,
    .on_enter   = home_enter,
    .on_leave   = home_leave,
    .on_destroy = NULL,
    .sub_pages  = NULL,
    .sub_count  = 0,
};
