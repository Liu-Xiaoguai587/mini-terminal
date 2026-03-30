#include "gui_page.h"

/* ── External page declarations ───────────────────────── */
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

/* ── Menu item click handler ──────────────────────────── */
static void menu_item_cb(lv_event_t *e) {
    const PageDef_t *page = lv_event_get_user_data(e);
    page_mgr_push(page);
}

/* ── on_create: build the menu list ───────────────────── */
static void menu_create(lv_obj_t *parent) {
    const PageDef_t *node = page_mgr_current();

    lv_obj_t *list = lv_list_create(parent);
    lv_obj_set_size(list, LV_PCT(100), LV_PCT(100));
    lv_obj_set_pos(list, 0, 0);
    lv_obj_set_style_pad_all(list, 0, 0);

    for (int i = 0; i < node->sub_count; i++) {
        lv_obj_t *btn = lv_list_add_btn(list, NULL, node->sub_pages[i]->title);
        lv_obj_add_event_cb(btn, menu_item_cb, LV_EVENT_CLICKED,
                            (void *)node->sub_pages[i]);
        lv_group_add_obj(lv_group_get_default(), btn);
    }
}

/* ── Main menu page definition ────────────────────────── */
const PageDef_t page_main_menu = {
    .title      = "Main Menu",
    .on_create  = menu_create,
    .on_enter   = NULL,
    .on_leave   = NULL,
    .on_destroy = NULL,
    .sub_pages  = menu_sub_pages,
    .sub_count  = sizeof(menu_sub_pages) / sizeof(menu_sub_pages[0]),
};
