#ifndef __GUI_PAGE_H__
#define __GUI_PAGE_H__

#include "lvgl.h"
#include <stdint.h>

typedef struct PageDef PageDef_t;

struct PageDef {
    const char     *title;

    /* Lifecycle callbacks (all optional, NULL = skip) */
    void           (*on_create)(lv_obj_t *parent);
    void           (*on_enter)(void);
    void           (*on_leave)(void);
    void           (*on_destroy)(void);

    /* Sub-pages for menu-type pages */
    const PageDef_t * const *sub_pages;
    uint8_t          sub_count;
};

void             page_mgr_init(const PageDef_t *root);
void             page_mgr_push(const PageDef_t *page);
void             page_mgr_pop(void);
const PageDef_t *page_mgr_current(void);

#endif /* __GUI_PAGE_H__ */
