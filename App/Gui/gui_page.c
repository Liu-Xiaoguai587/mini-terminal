#include "gui_page.h"

#define MAX_PAGE_STACK 10

static struct {
    const PageDef_t *stack[MAX_PAGE_STACK];
    lv_obj_t        *current_screen;
    lv_obj_t        *pending_del;      /* screen to delete on next transition */
    int8_t           top;
} s_mgr;

/* ── Internal: create & load the page at the top of the stack ── */
static void load_page_at_top(void) {
    /* Delete screen left over from the PREVIOUS transition (safe here) */
    if (s_mgr.pending_del) {
        lv_obj_del(s_mgr.pending_del);
        s_mgr.pending_del = NULL;
    }

    lv_obj_t *old_scr = s_mgr.current_screen;

    /* 1. Clear group and exit edit mode so new page starts in nav mode */
    lv_group_t *g = lv_group_get_default();
    lv_group_remove_all_objs(g);
    lv_group_set_editing(g, false);

    /* 2. Create screen and call on_create (widgets added to clean group) */
    lv_obj_t *scr = lv_obj_create(NULL);
    s_mgr.current_screen = scr;

    const PageDef_t *page = s_mgr.stack[s_mgr.top];
    if (page->on_create) page->on_create(scr);

    /* 3. Make it visible, then notify page */
    lv_scr_load(scr);
    if (page->on_enter) page->on_enter();

    /* 4. Defer old screen deletion to the NEXT transition.
     *    This avoids use-after-free when called from an event callback
     *    on a widget that belongs to old_scr. */
    s_mgr.pending_del = old_scr;
}

/* ── Public API ──────────────────────────────────────────────── */

void page_mgr_init(const PageDef_t *root) {
    /* Delete the default screen that lv_init() created to reclaim memory */
    lv_obj_t *default_scr = lv_scr_act();

    s_mgr.top            = -1;
    s_mgr.current_screen = NULL;
    s_mgr.pending_del    = NULL;
    page_mgr_push(root);

    /* Now our root screen is active; safe to delete the old default screen */
    if (default_scr && default_scr != s_mgr.current_screen) {
        lv_obj_del(default_scr);
    }
}

void page_mgr_push(const PageDef_t *page) {
    if (s_mgr.top >= MAX_PAGE_STACK - 1) return;

    /* Leave current page */
    if (s_mgr.top >= 0) {
        const PageDef_t *cur = s_mgr.stack[s_mgr.top];
        if (cur->on_leave) cur->on_leave();
    }

    s_mgr.top++;
    s_mgr.stack[s_mgr.top] = page;
    load_page_at_top();
}

void page_mgr_pop(void) {
    if (s_mgr.top <= 0) return;  /* don't pop root */

    const PageDef_t *cur = s_mgr.stack[s_mgr.top];
    if (cur->on_leave)   cur->on_leave();
    if (cur->on_destroy) cur->on_destroy();

    s_mgr.top--;
    load_page_at_top();
}

const PageDef_t *page_mgr_current(void) {
    if (s_mgr.top < 0) return NULL;
    return s_mgr.stack[s_mgr.top];
}
