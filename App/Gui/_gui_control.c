#include "gui_control.h"
#include "lvgl.h"

#define MAX_PAGE_LAYER 10

static struct {
    uint8_t current_node;
    PageNode_t *node_stack[MAX_PAGE_LAYER];
} Gui_Controler;

extern void gui_menu(void);
extern void gui_test(void);
extern void gui_test2(void);

static PageNode_t page_test1 = {
    .title    = "Test Page 1",
    .instance = gui_test,
    .sub_node = NULL,
    .sub_size = 0
};

static PageNode_t page_test2 = {
    .title    = "Test Page 2",
    .instance = gui_test2,
    .sub_node = NULL,
    .sub_size = 0
};

static PageNode_t *main_sub_nodes[] = { &page_test1, &page_test2 };

static PageNode_t main_page = {
    .title    = "Main Page",
    .instance = gui_menu,
    .sub_node = main_sub_nodes,
    .sub_size = 2
};

static void load_page(PageNode_t *page) {
    lv_obj_clean(lv_scr_act());
    lv_group_remove_all_objs(lv_group_get_default());
    page->instance();
}

void gui_control_init(void) {
    Gui_Controler.current_node = 0;
    Gui_Controler.node_stack[0] = &main_page;
}

void gui_show_page(void) {
    load_page(Gui_Controler.node_stack[Gui_Controler.current_node]);
}

void gui_push_page(PageNode_t *page) {
    if (Gui_Controler.current_node >= MAX_PAGE_LAYER - 1) return;
    Gui_Controler.current_node++;
    Gui_Controler.node_stack[Gui_Controler.current_node] = page;
    load_page(page);
}

void gui_pop_page(void) {
    if (Gui_Controler.current_node == 0) return;
    Gui_Controler.current_node--;
    load_page(Gui_Controler.node_stack[Gui_Controler.current_node]);
}

PageNode_t *gui_get_current_node(void) {
    return Gui_Controler.node_stack[Gui_Controler.current_node];
}
