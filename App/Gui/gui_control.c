#include <stdint.h>

#define NULL    ((void *)0)
#define MAX_PAGE_LAYER 10

typedef void(*GuiPageFunc_t)(void);


typedef struct PageNode {
    char *title;
    // gui instance
    GuiPageFunc_t instance; 

    struct PageNode **sub_node;
    uint8_t sub_size;
} PageNode_t;

struct {
    PageNode_t *top_page_node;
    // struct GuiNode *current_node;
    uint8_t current_node;
    PageNode_t *node_stack[MAX_PAGE_LAYER]; // node history

} Gui_Controler;




extern void gui_test(void);

static PageNode_t main_page = {
    .title = "Main Page",
    .instance = gui_test,
    .sub_node = NULL,
    .sub_size = 0
}; 

void gui_control_init() {
    Gui_Controler.top_page_node = &main_page;
    Gui_Controler.current_node = 0;
    for (int i = 0; i < MAX_PAGE_LAYER; i++) {
        Gui_Controler.node_stack[i] = NULL;
    }
}

void gui_show_page() {
    Gui_Controler.top_page_node->instance();
}
