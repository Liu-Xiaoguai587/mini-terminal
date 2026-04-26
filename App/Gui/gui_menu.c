#include "gui_page.h"

/* ── Sub-page declarations ─────────────────────────────────── */
extern const PageDef_t page_test;
extern const PageDef_t page_btn_test;
extern const PageDef_t page_sensor;
extern const PageDef_t page_clock;

static const PageDef_t * const menu_sub_pages[] = {
    &page_test,
    &page_btn_test,
    &page_sensor,
    &page_clock,
};
#define ITEM_COUNT ((int)(sizeof(menu_sub_pages) / sizeof(menu_sub_pages[0])))

/* ── State ─────────────────────────────────────────────────── */
static int       s_cur   = 0;       /* survives page push/pop */
static lv_obj_t *s_lbl_left;
static lv_obj_t *s_lbl_title;
static lv_obj_t *s_lbl_right;
static lv_obj_t *s_lbl_index;

/* ── Update display widgets to reflect s_cur ───────────────── */
static void update_card(void) {
    lv_label_set_text(s_lbl_left,  LV_SYMBOL_LEFT);
    lv_label_set_text(s_lbl_title, menu_sub_pages[s_cur]->title);
    lv_label_set_text(s_lbl_right, LV_SYMBOL_RIGHT);
    lv_label_set_text_fmt(s_lbl_index, "%d / %d", s_cur + 1, ITEM_COUNT);
}

/* ── Card event handler ────────────────────────────────────── */
static void card_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_KEY) {
        uint32_t key = *(uint32_t *)lv_event_get_param(e);
        if (key == LV_KEY_RIGHT) {
            s_cur = (s_cur + 1) % ITEM_COUNT;
            update_card();
        } else if (key == LV_KEY_LEFT) {
            s_cur = (s_cur - 1 + ITEM_COUNT) % ITEM_COUNT;
            update_card();
        } else if (key == LV_KEY_ENTER) {
            page_mgr_push(menu_sub_pages[s_cur]);
        }
    } else if (code == LV_EVENT_CLICKED) {
        /* Encoder center-press in edit mode also fires CLICKED */
        page_mgr_push(menu_sub_pages[s_cur]);
    }
}

/* ── on_create ─────────────────────────────────────────────── */
static void menu_create(lv_obj_t *parent) {
    /* Full-screen card — plain container, no default padding/border */
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_size(card, LV_PCT(100), LV_PCT(100));
    lv_obj_set_pos(card, 0, 0);
    lv_obj_set_style_radius(card, 0, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_pad_all(card, 0, 0);
    lv_obj_add_flag(card, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(card, card_event_cb, LV_EVENT_ALL, NULL);
    lv_group_add_obj(lv_group_get_default(), card);

    /* Left navigation arrow */
    s_lbl_left = lv_label_create(card);
    lv_obj_align(s_lbl_left, LV_ALIGN_LEFT_MID, 8, 0);

    /* Item title — centered, slightly above mid */
    s_lbl_title = lv_label_create(card);
    lv_label_set_long_mode(s_lbl_title, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(s_lbl_title, 100);   /* leave room for arrows */
    lv_obj_set_style_text_align(s_lbl_title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(s_lbl_title, LV_ALIGN_CENTER, 0, -8);

    /* Right navigation arrow */
    s_lbl_right = lv_label_create(card);
    lv_obj_align(s_lbl_right, LV_ALIGN_RIGHT_MID, -8, 0);

    /* "n / total" indicator at the bottom */
    s_lbl_index = lv_label_create(card);
    lv_obj_set_style_text_color(s_lbl_index, lv_color_hex(0x888888), 0);
    lv_obj_align(s_lbl_index, LV_ALIGN_BOTTOM_MID, 0, -6);

    update_card();

    /*
     * Put the group into edit mode so encoder rotation sends
     * LV_KEY_LEFT / LV_KEY_RIGHT to the focused card widget
     * instead of cycling focus between objects.
     */
    lv_group_set_editing(lv_group_get_default(), true);
}

/* ── Page definition ───────────────────────────────────────── */
const PageDef_t page_main_menu = {
    .title      = "Main Menu",
    .on_create  = menu_create,
    .on_enter   = NULL,
    .on_leave   = NULL,
    .on_destroy = NULL,
    .sub_pages  = menu_sub_pages,
    .sub_count  = ITEM_COUNT,
};
