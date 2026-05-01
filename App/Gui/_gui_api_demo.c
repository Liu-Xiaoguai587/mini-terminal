#include "gui_page.h"
#include "net_data.h"
#include "FreeRTOS.h"
#include "task.h"

extern volatile uint8_t  g_wifi_stage;
extern volatile int16_t  g_wifi_last_ret;
extern volatile uint8_t  g_esp_http_stage;
extern volatile uint16_t g_esp_recv_len;
extern volatile int16_t  g_esp_http_status;

static lv_obj_t   *lbl_hint;
static lv_obj_t   *lbl_result;
static lv_timer_t *poll_timer;

#define API_DEMO_VER "api-demo v1.0-cache"

static void update_display(lv_timer_t *t) {
    NetData_t nd;
    NetApiStatus_t api;
    uint32_t age_sec;

    (void)t;

    nd = net_data_get();
    api = net_data_get_api_status();

    if (nd.wifi_state != ESP_WIFI_CONNECTED) {
        lv_label_set_text(lbl_hint, "WiFi not ready");
    } else if (nd.http_busy) {
        lv_label_set_text(lbl_hint, "Updating...");
    } else if (nd.last_http_ok) {
        lv_label_set_text_fmt(lbl_hint, "HTTP %d recv:%u",
                              nd.last_http_status,
                              (unsigned)g_esp_recv_len);
    } else {
        lv_label_set_text_fmt(lbl_hint, "Last failed (%d)", nd.last_http_status);
    }

    if (!api.valid) {
        lv_label_set_text_fmt(lbl_result,
                              "Waiting data\n"
                              "w:%u/%d e:%u %u %d",
                              (unsigned)g_wifi_stage,
                              (int)g_wifi_last_ret,
                              (unsigned)g_esp_http_stage,
                              (unsigned)g_esp_recv_len,
                              (int)g_esp_http_status);
        return;
    }

    age_sec = (uint32_t)((xTaskGetTickCount() - api.last_update) / configTICK_RATE_HZ);

    lv_label_set_text_fmt(lbl_result,
                          "dev:%s\n"
                          "city:%s temp:%d\n"
                          "BTC:%lu\n"
                          "ETH:%lu\n"
                          "online:%s age:%lus",
                          api.device,
                          api.city,
                          (int)api.temp,
                          (unsigned long)api.btc_usd,
                          (unsigned long)api.eth_usd,
                          api.online ? "true" : "false",
                          (unsigned long)age_sec);
}

static void api_create(lv_obj_t *parent) {
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_size(card, LV_PCT(100), LV_PCT(100));
    lv_obj_set_pos(card, 0, 0);
    lv_obj_set_style_radius(card, 0, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_pad_all(card, 4, 0);

    lbl_hint = lv_label_create(card);
    lv_label_set_text(lbl_hint, API_DEMO_VER);
    lv_obj_align(lbl_hint, LV_ALIGN_TOP_MID, 0, 4);

    lbl_result = lv_label_create(card);
    lv_label_set_text(lbl_result, "");
    lv_label_set_long_mode(lbl_result, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(lbl_result, 150);
    lv_obj_align(lbl_result, LV_ALIGN_TOP_LEFT, 2, 24);
}

static void api_enter(void) {
    if (!poll_timer) {
        poll_timer = lv_timer_create(update_display, 1000, NULL);
    }
    update_display(NULL);
}

static void api_leave(void) {
    if (poll_timer) {
        lv_timer_del(poll_timer);
        poll_timer = NULL;
    }
}

const PageDef_t page_api_demo = {
    .title      = API_DEMO_VER,
    .on_create  = api_create,
    .on_enter   = api_enter,
    .on_leave   = api_leave,
    .on_destroy = NULL,
    .sub_pages  = NULL,
    .sub_count  = 0,
};
