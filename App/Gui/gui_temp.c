#include "gui_page.h"
#include "gui_theme.h"
#include "sensor_data.h"

static lv_obj_t   *s_temp_value;
static lv_obj_t   *s_hum_value;
static lv_obj_t   *s_temp_status;
static lv_timer_t *s_temp_timer;

static void temp_update_labels(lv_timer_t *timer) {
    SensorData_t d;
    int t_int;
    int t_dec;
    int h_int;
    int h_dec;

    (void)timer;
    if (!s_temp_value || !s_hum_value) return;

    d = sensor_data_get();
    if (!d.aht10_ok) {
        lv_label_set_text(s_temp_value, "--C");
        lv_label_set_text(s_hum_value, "--%");
        if (s_temp_status) lv_label_set_text(s_temp_status, "> SYS_WAIT");
        return;
    }

    t_int = (int)d.temperature;
    t_dec = ((int)(d.temperature * 10.0f)) % 10;
    h_int = (int)d.humidity;
    h_dec = ((int)(d.humidity * 10.0f)) % 10;
    if (t_dec < 0) t_dec = -t_dec;
    if (h_dec < 0) h_dec = -h_dec;

    lv_label_set_text_fmt(s_temp_value, "%d.%dC", t_int, t_dec);
    lv_label_set_text_fmt(s_hum_value, "%d.%d%%", h_int, h_dec);
    if (s_temp_status) lv_label_set_text(s_temp_status, "> SYS_OK");
}

static void temp_metric_panel(lv_obj_t *parent,
                              lv_align_t align,
                              lv_coord_t x,
                              const char *name,
                              const char *icon,
                              const char *value) {
    lv_obj_t *panel;
    lv_obj_t *tag;
    lv_obj_t *main;

    panel = gui_panel(parent, 66, 42);
    lv_obj_align(panel, align, x, 0);

    tag = gui_label(panel, name, lv_color_hex(GUI_AMBER));
    gui_clip_label(tag, 52);
    lv_obj_set_style_bg_color(tag, lv_color_hex(GUI_BLACK), 0);
    lv_obj_set_style_bg_opa(tag, LV_OPA_COVER, 0);
    lv_obj_align(tag, LV_ALIGN_TOP_LEFT, 0, -3);

    main = gui_label(panel, icon, lv_color_hex(GUI_AMBER));
    lv_obj_align(main, LV_ALIGN_LEFT_MID, 4, 4);

    main = gui_label(panel, value, lv_color_hex(GUI_AMBER));
    lv_obj_set_style_text_font(main, GUI_FONT_BIG, 0);
    lv_obj_align(main, LV_ALIGN_RIGHT_MID, -4, 4);

    if (name[0] == 'T') {
        s_temp_value = main;
    } else {
        s_hum_value = main;
    }
}

void gui_temp_render(lv_obj_t *parent) {
    gui_plain_obj(parent, lv_color_hex(GUI_BLACK));
    temp_metric_panel(parent, LV_ALIGN_LEFT_MID, 4, "TEMP", LV_SYMBOL_CHARGE, "--C");
    temp_metric_panel(parent, LV_ALIGN_RIGHT_MID, -4, "HUMID", LV_SYMBOL_TINT, "--%");
    s_temp_status = gui_status_cursor(parent, "> SYS_WAIT");
    temp_update_labels(NULL);
}

static void temp_create(lv_obj_t *parent) {
    lv_obj_t *content = gui_page_content(parent, "[ENV_SENSORS]");
    gui_temp_render(content);
}

static void temp_enter(void) {
    if (!s_temp_timer) {
        s_temp_timer = lv_timer_create(temp_update_labels, 1000, NULL);
    }
    temp_update_labels(NULL);
}

static void temp_leave(void) {
    if (s_temp_timer) {
        lv_timer_del(s_temp_timer);
        s_temp_timer = NULL;
    }
}

const PageDef_t page_temp = {
    .title      = "[ENV_SENSORS]",
    .on_create  = temp_create,
    .on_enter   = temp_enter,
    .on_leave   = temp_leave,
    .on_destroy = NULL,
    .sub_pages  = NULL,
    .sub_count  = 0,
};
