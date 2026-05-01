#include "gui_page.h"
#include "gui_theme.h"
#include "net_data.h"

static lv_obj_t   *s_web3_sync;
static lv_obj_t   *s_web3_blk;
static lv_obj_t   *s_web3_btc;
static lv_obj_t   *s_web3_eth;
static lv_timer_t *s_web3_timer;

static void web3_update_labels(lv_timer_t *timer) {
    NetData_t nd;
    NetApiStatus_t api;

    (void)timer;
    if (!s_web3_sync || !s_web3_btc || !s_web3_eth) return;

    nd = net_data_get();
    api = net_data_get_api_status();

    if (nd.http_busy) {
        lv_label_set_text(s_web3_sync, LV_SYMBOL_PLAY " SYNCING");
    } else if (api.valid) {
        lv_label_set_text(s_web3_sync, LV_SYMBOL_PLAY " SYNC: OK");
    } else {
        lv_label_set_text(s_web3_sync, LV_SYMBOL_PLAY " WAIT");
    }

    if (!api.valid) {
        lv_label_set_text(s_web3_blk, "HTTP:--");
        lv_label_set_text(s_web3_btc, "$--");
        lv_label_set_text(s_web3_eth, "$--");
        return;
    }

    lv_label_set_text_fmt(s_web3_blk, "HTTP:%d", (int)nd.last_http_status);
    lv_label_set_text_fmt(s_web3_btc, "$%lu", (unsigned long)api.btc_usd);
    lv_label_set_text_fmt(s_web3_eth, "$%lu", (unsigned long)api.eth_usd);
}

static void web3_ticker_row(lv_obj_t *parent,
                            lv_coord_t y,
                            const char *name,
                            const char *value,
                            uint8_t active) {
    lv_obj_t *row;
    lv_obj_t *left;
    lv_obj_t *right;

    row = lv_obj_create(parent);
    lv_obj_set_size(row, 138, 18);
    lv_obj_align(row, LV_ALIGN_TOP_MID, 0, y);
    gui_plain_obj(row, active ? lv_color_hex(GUI_AMBER)
                              : lv_color_hex(GUI_BLACK));
    lv_obj_set_style_border_width(row, 1, 0);
    lv_obj_set_style_border_color(row, lv_color_hex(GUI_AMBER), 0);
    lv_obj_set_style_border_opa(row, active ? LV_OPA_COVER : LV_OPA_50, 0);

    left = gui_label(row, name, active ? lv_color_hex(GUI_BLACK)
                                      : lv_color_hex(GUI_AMBER));
    gui_clip_label(left, 52);
    lv_obj_align(left, LV_ALIGN_LEFT_MID, 4, 0);

    right = gui_label(row, value, active ? lv_color_hex(GUI_BLACK)
                                        : lv_color_hex(GUI_AMBER));
    if (name[2] == 'B') {
        s_web3_btc = right;
    } else if (name[2] == 'E') {
        s_web3_eth = right;
    }
    gui_clip_label(right, 64);
    lv_obj_set_style_text_align(right, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_align(right, LV_ALIGN_RIGHT_MID, -4, 0);
}

void gui_web3_render(lv_obj_t *parent) {
    lv_obj_t *sync;
    lv_obj_t *blk;

    gui_plain_obj(parent, lv_color_hex(GUI_BLACK));

    sync = gui_label(parent, LV_SYMBOL_PLAY " SYNC: OK", lv_color_hex(GUI_AMBER));
    s_web3_sync = sync;
    gui_clip_label(sync, 72);
    lv_obj_align(sync, LV_ALIGN_TOP_LEFT, 6, 4);

    blk = gui_label(parent, "BLK:840K", lv_color_hex(GUI_DIM));
    s_web3_blk = blk;
    gui_clip_label(blk, 62);
    lv_obj_set_style_text_align(blk, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_align(blk, LV_ALIGN_TOP_RIGHT, -5, 4);

    web3_ticker_row(parent, 24, "> BTC", "$64231", 1);
    web3_ticker_row(parent, 46, "  ETH", "$3452", 0);
    // web3_ticker_row(parent, 68, "  SOL", "$142", 0);
    web3_update_labels(NULL);
}

static void web3_create(lv_obj_t *parent) {
    lv_obj_t *content = gui_page_content(parent, "[WEB3_TICKER]");
    gui_web3_render(content);
}

static void web3_enter(void) {
    if (!s_web3_timer) {
        s_web3_timer = lv_timer_create(web3_update_labels, 1000, NULL);
    }
    web3_update_labels(NULL);
}

static void web3_leave(void) {
    if (s_web3_timer) {
        lv_timer_del(s_web3_timer);
        s_web3_timer = NULL;
    }
}

const PageDef_t page_web3 = {
    .title      = "[WEB3_TICKER]",
    .on_create  = web3_create,
    .on_enter   = web3_enter,
    .on_leave   = web3_leave,
    .on_destroy = NULL,
    .sub_pages  = NULL,
    .sub_count  = 0,
};
