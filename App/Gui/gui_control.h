#ifndef __GUI_CONTROL_H__
#define __GUI_CONTROL_H__

#include <stdint.h>

typedef void (*GuiPageFunc_t)(void);

typedef struct PageNode {
    char *title;
    GuiPageFunc_t instance;
    struct PageNode **sub_node;
    uint8_t sub_size;
} PageNode_t;

void gui_control_init(void);
void gui_show_page(void);
void gui_push_page(PageNode_t *page);
void gui_pop_page(void);
PageNode_t *gui_get_current_node(void);

#endif // __GUI_CONTROL_H__
