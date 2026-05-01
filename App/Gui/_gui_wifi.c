#include "gui_page.h"
#include "net_data.h"

static lv_obj_t   *lbl_status;
static lv_obj_t   *lbl_ip;
static lv_timer_t *poll_timer;

static const char *wifi_state_str(ESP_WifiState_t s) {
    switch (s) {
        case ESP_WIFI_CONNECTED:    return "Connected";
        case ESP_WIFI_CONNECTING:   return "Connecting...";
        case ESP_WIFI_DISCONNECTED: return "Disconnected";
        case ESP_WIFI_ERROR:        return "Error";
        default:                    return "Unknown";
    }
}

static void update_display(lv_timer_t *t) {
    (void)t;
    NetData_t nd = net_data_get();

    lv_label_set_text_fmt(lbl_status, "WiFi: %s", wifi_state_str(nd.wifi_state));

    if (nd.wifi_state == ESP_WIFI_CONNECTED && nd.ip_addr[0]) {
        lv_label_set_text_fmt(lbl_ip, "IP: %s", nd.ip_addr);
    } else {
        lv_label_set_text(lbl_ip, "IP: --");
    }
}

static void wifi_create(lv_obj_t *parent) {
    lbl_status = lv_label_create(parent);
    lv_label_set_text(lbl_status, "WiFi: --");
    lv_obj_align(lbl_status, LV_ALIGN_TOP_MID, 0, 20);

    lbl_ip = lv_label_create(parent);
    lv_label_set_text(lbl_ip, "IP: --");
    lv_obj_align(lbl_ip, LV_ALIGN_TOP_MID, 0, 50);
}

static void wifi_enter(void) {
    poll_timer = lv_timer_create(update_display, 1000, NULL);
    update_display(NULL);
}

static void wifi_leave(void) {
    if (poll_timer) {
        lv_timer_del(poll_timer);
        poll_timer = NULL;
    }
}

const PageDef_t page_wifi = {
    .title      = "WiFi",
    .on_create  = wifi_create,
    .on_enter   = wifi_enter,
    .on_leave   = wifi_leave,
    .on_destroy = NULL,
    .sub_pages  = NULL,
    .sub_count  = 0,
};
