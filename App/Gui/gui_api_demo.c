#include "gui_page.h"
#include "net_data.h"
#include "FreeRTOS.h"
#include "queue.h"
#include <stdio.h>

/* Import wifi command queue and types from task_wifi */
extern QueueHandle_t wifi_cmd_queue;

/* Mirror the command types from task_wifi.c */
typedef enum {
    WIFI_CMD_CONNECT,
    WIFI_CMD_DISCONNECT,
    WIFI_CMD_HTTP_GET,
    WIFI_CMD_HTTP_POST,
} WifiCmdType_t;

typedef struct {
    WifiCmdType_t  type;
    const char    *host;
    uint16_t       port;
    const char    *path;
    const char    *content_type;
    const char    *post_body;
    uint16_t       post_body_len;
} WifiCmd_t;

static lv_obj_t   *lbl_hint;
static lv_obj_t   *lbl_result;
static lv_timer_t *poll_timer;
static uint8_t     s_requested;

static void set_json_result_text(void) {
    NetJsonData_t json = net_data_get_json();
    char text[192];
    uint16_t pos = 0;
    uint8_t i;

    if (!json.valid || json.field_count == 0) {
        NetData_t nd = net_data_get();
        lv_label_set_text(lbl_result, nd.last_http_body);
        return;
    }

    for (i = 0; i < json.field_count && pos < sizeof(text) - 1; i++) {
        NetJsonField_t *field = &json.fields[i];
        int n;

        if (field->type == NET_JSON_TYPE_NUMBER) {
            int32_t scaled = (int32_t)(field->number * 100.0);
            int32_t whole;
            int32_t frac;
            const char *sign = "";

            if (scaled < 0) {
                scaled = -scaled;
                sign = "-";
            }
            whole = scaled / 100;
            frac = scaled % 100;
            n = snprintf(text + pos, sizeof(text) - pos, "%s: %s%ld.%02ld\n",
                         field->key, sign, (long)whole, (long)frac);
        } else {
            n = snprintf(text + pos, sizeof(text) - pos, "%s: %s\n",
                         field->key, field->value);
        }

        if (n <= 0) break;
        if ((uint16_t)n >= sizeof(text) - pos) {
            pos = sizeof(text) - 1;
            break;
        }
        pos += (uint16_t)n;
    }

    text[pos] = '\0';
    lv_label_set_text(lbl_result, text);
}

/* Demo API endpoint */
#define DEMO_HOST  "httpbin.org"
#define DEMO_PORT  80
#define DEMO_PATH  "/get"

static void update_display(lv_timer_t *t) {
    (void)t;
    NetData_t nd = net_data_get();

    if (nd.http_busy) {
        lv_label_set_text(lbl_hint, "Requesting...");
    } else if (s_requested && nd.last_http_ok) {
        lv_label_set_text_fmt(lbl_hint, "HTTP %d", nd.last_http_status);
        set_json_result_text();
    } else if (s_requested) {
        lv_label_set_text(lbl_hint, "Request failed");
        lv_label_set_text(lbl_result, "");
    } else {
        lv_label_set_text(lbl_hint, "Press to fetch");
    }
}

static void card_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_KEY) {
        uint32_t key = *(uint32_t *)lv_event_get_param(e);
        if (key == LV_KEY_ENTER) {
            /* Send HTTP GET command to wifi task */
            WifiCmd_t cmd = {0};
            cmd.type = WIFI_CMD_HTTP_GET;
            cmd.host = DEMO_HOST;
            cmd.port = DEMO_PORT;
            cmd.path = DEMO_PATH;
            if (wifi_cmd_queue) {
                xQueueSend(wifi_cmd_queue, &cmd, 0);
                s_requested = 1;
                lv_label_set_text(lbl_hint, "Requesting...");
                lv_label_set_text(lbl_result, "");
            }
        }
    } else if (code == LV_EVENT_CLICKED) {
        WifiCmd_t cmd = {0};
        cmd.type = WIFI_CMD_HTTP_GET;
        cmd.host = DEMO_HOST;
        cmd.port = DEMO_PORT;
        cmd.path = DEMO_PATH;
        if (wifi_cmd_queue) {
            xQueueSend(wifi_cmd_queue, &cmd, 0);
            s_requested = 1;
            lv_label_set_text(lbl_hint, "Requesting...");
            lv_label_set_text(lbl_result, "");
        }
    }
}

static void api_create(lv_obj_t *parent) {
    /* Clickable container */
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_size(card, LV_PCT(100), LV_PCT(100));
    lv_obj_set_pos(card, 0, 0);
    lv_obj_set_style_radius(card, 0, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_pad_all(card, 4, 0);
    lv_obj_add_flag(card, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(card, card_event_cb, LV_EVENT_ALL, NULL);
    lv_group_add_obj(lv_group_get_default(), card);

    lbl_hint = lv_label_create(card);
    lv_label_set_text(lbl_hint, "Press to fetch");
    lv_obj_align(lbl_hint, LV_ALIGN_TOP_MID, 0, 4);

    lbl_result = lv_label_create(card);
    lv_label_set_text(lbl_result, "");
    lv_label_set_long_mode(lbl_result, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(lbl_result, 150);
    lv_obj_align(lbl_result, LV_ALIGN_TOP_LEFT, 2, 24);

    lv_group_set_editing(lv_group_get_default(), true);
}

static void api_enter(void) {
    s_requested = 0;
    poll_timer = lv_timer_create(update_display, 500, NULL);
}

static void api_leave(void) {
    if (poll_timer) {
        lv_timer_del(poll_timer);
        poll_timer = NULL;
    }
}

const PageDef_t page_api_demo = {
    .title      = "API Demo",
    .on_create  = api_create,
    .on_enter   = api_enter,
    .on_leave   = api_leave,
    .on_destroy = NULL,
    .sub_pages  = NULL,
    .sub_count  = 0,
};
