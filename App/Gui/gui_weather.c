#include "gui_page.h"
#include "gui_theme.h"
#include "net_data.h"

static lv_obj_t   *s_weather_temp;
static lv_obj_t   *s_weather_stat;
static lv_obj_t   *s_weather_hum;
static lv_obj_t   *s_weather_wind;
static lv_obj_t   *s_weather_status;
static lv_timer_t *s_weather_timer;

static void weather_update_labels(lv_timer_t *timer) {
    NetApiStatus_t api;
    double hum;
    double wind;

    (void)timer;
    if (!s_weather_temp) return;

    api = net_data_get_api_status();
    if (!api.valid) {
        lv_label_set_text(s_weather_temp, "--C");
        lv_label_set_text(s_weather_stat, "STAT: WAIT");
        lv_label_set_text(s_weather_hum, "CITY: --");
        lv_label_set_text(s_weather_wind, "NET : --");
        if (s_weather_status) lv_label_set_text(s_weather_status, "> Updating");
        return;
    }

    lv_label_set_text_fmt(s_weather_temp, "%dC", (int)api.temp);
    lv_label_set_text_fmt(s_weather_stat, "CITY: %s", api.city[0] ? api.city : "--");
    lv_label_set_text_fmt(s_weather_wind, "NET : %s", api.online ? "ONLINE" : "OFFLINE");

    if (net_data_json_get_number("humidity", &hum)) {
        lv_label_set_text_fmt(s_weather_hum, "HUM : %d%%", (int)hum);
    } else if (net_data_json_get_number("wind", &wind)) {
        lv_label_set_text_fmt(s_weather_hum, "WIND: %dK/H", (int)wind);
    } else {
        lv_label_set_text(s_weather_hum, "HUM : --");
    }

    if (s_weather_status) lv_label_set_text(s_weather_status, "> API_OK");
}

void gui_weather_render(lv_obj_t *parent) {
    lv_obj_t *cloud;
    lv_obj_t *temp;
    lv_obj_t *sep;
    lv_obj_t *row;

    gui_plain_obj(parent, lv_color_hex(GUI_BLACK));
    lv_obj_set_style_pad_all(parent, 4, 0);

    cloud = gui_label(parent, LV_SYMBOL_GPS, lv_color_hex(GUI_AMBER));
    lv_obj_set_style_text_font(cloud, GUI_FONT_BIG, 0);
    lv_obj_align(cloud, LV_ALIGN_LEFT_MID, 16, -18);

    temp = gui_label(parent, "22C", lv_color_hex(GUI_AMBER));
    s_weather_temp = temp;
    lv_obj_set_style_text_font(temp, GUI_FONT_BIG, 0);
    lv_obj_align(temp, LV_ALIGN_LEFT_MID, 12, 2);

    sep = lv_obj_create(parent);
    lv_obj_set_size(sep, 1, 46);
    gui_plain_obj(sep, lv_color_hex(GUI_DIM));
    lv_obj_align(sep, LV_ALIGN_LEFT_MID, 56, -8);

    row = gui_label(parent, "STAT: CLOUDY", lv_color_hex(GUI_AMBER));
    s_weather_stat = row;
    lv_obj_set_style_text_font(row, GUI_FONT_SMALL, 0);
    gui_clip_label(row, 80);
    lv_obj_align(row, LV_ALIGN_TOP_RIGHT, -2, 4);

    row = gui_label(parent, "HUM : 65%", lv_color_hex(GUI_AMBER));
    s_weather_hum = row;
    lv_obj_set_style_text_font(row, GUI_FONT_SMALL, 0);
    gui_clip_label(row, 80);
    lv_obj_align(row, LV_ALIGN_TOP_RIGHT, -2, 20);

    row = gui_label(parent, "WIND: 12K/H", lv_color_hex(GUI_AMBER));
    s_weather_wind = row;
    lv_obj_set_style_text_font(row, GUI_FONT_SMALL, 0);
    gui_clip_label(row, 80);
    lv_obj_align(row, LV_ALIGN_TOP_RIGHT, -2, 36);

    s_weather_status = gui_status_cursor(parent, "> Updating");
    weather_update_labels(NULL);
}

static void weather_create(lv_obj_t *parent) {
    lv_obj_t *content = gui_page_content(parent, "[WEATHER_DATA]");
    gui_weather_render(content);
}

static void weather_enter(void) {
    if (!s_weather_timer) {
        s_weather_timer = lv_timer_create(weather_update_labels, 1000, NULL);
    }
    weather_update_labels(NULL);
}

static void weather_leave(void) {
    if (s_weather_timer) {
        lv_timer_del(s_weather_timer);
        s_weather_timer = NULL;
    }
}

const PageDef_t page_weather = {
    .title      = "[WEATHER_DATA]",
    .on_create  = weather_create,
    .on_enter   = weather_enter,
    .on_leave   = weather_leave,
    .on_destroy = NULL,
    .sub_pages  = NULL,
    .sub_count  = 0,
};
