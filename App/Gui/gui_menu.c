#include "lvgl.h"
#include "gui_control.h"

static void menu_item_cb(lv_event_t *e) {
    PageNode_t *page = lv_event_get_user_data(e);
    gui_push_page(page);
}

void gui_menu(void) {
    PageNode_t *node = gui_get_current_node();

    lv_obj_t *list = lv_list_create(lv_scr_act());
    lv_obj_set_size(list, LV_PCT(100), LV_PCT(100));
    lv_obj_set_pos(list, 0, 0);
    lv_obj_set_style_pad_all(list, 0, 0);

    for (int i = 0; i < node->sub_size; i++) {
        lv_obj_t *btn = lv_list_add_btn(list, NULL, node->sub_node[i]->title);
        lv_obj_add_event_cb(btn, menu_item_cb, LV_EVENT_CLICKED, node->sub_node[i]);
        lv_group_add_obj(lv_group_get_default(), btn);
    }
}
