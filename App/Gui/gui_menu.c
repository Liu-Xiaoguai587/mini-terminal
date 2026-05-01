#include "gui_page.h"
#include "gui_theme.h"

/* Sub-page declarations */
extern const PageDef_t page_home;
extern const PageDef_t page_temp;
extern const PageDef_t page_weather;
extern const PageDef_t page_web3;
extern const PageDef_t page_config;

extern void gui_home_render(lv_obj_t *parent);
extern void gui_temp_render(lv_obj_t *parent);
extern void gui_weather_render(lv_obj_t *parent);
extern void gui_web3_render(lv_obj_t *parent);
extern void gui_config_render(lv_obj_t *parent);
extern void gui_config_preview(lv_obj_t *parent);

static const PageDef_t * const menu_sub_pages[] = {
    &page_home,
    &page_temp,
    &page_weather,
    &page_web3,
    &page_config,
};
#define ITEM_COUNT ((int)(sizeof(menu_sub_pages) / sizeof(menu_sub_pages[0])))

static const char * const menu_nav_icons[] = {
    LV_SYMBOL_HOME,
    LV_SYMBOL_CHARGE,
    LV_SYMBOL_GPS,
    LV_SYMBOL_LOOP,
    LV_SYMBOL_SETTINGS,
};
#define NAV_ICON_COUNT ((int)(sizeof(menu_nav_icons) / sizeof(menu_nav_icons[0])))

/* Layout */
#define MENU_TOP_H       GUI_TOP_H
#define MENU_NAV_H       GUI_NAV_H
#define MENU_ACCENT      GUI_AMBER
#define MENU_TEXT        GUI_AMBER
#define MENU_TEXT_MUTED  GUI_DIM
#define MENU_BG          GUI_BLACK
#define MENU_BAR_BG      GUI_BLACK
#define MENU_PREVIEW_BG  GUI_BLACK

/* State */
static int       s_cur = 0;         /* survives page push/pop */
static lv_obj_t *s_lbl_title;
static lv_obj_t *s_preview;
static lv_obj_t *s_nav_items[ITEM_COUNT];
static lv_obj_t *s_nav_icons[ITEM_COUNT];
static lv_timer_t *s_preview_timer;
static uint8_t   s_entering;

static const char *get_nav_icon(int index) {
    if (index < NAV_ICON_COUNT) {
        return menu_nav_icons[index];
    }

    return LV_SYMBOL_FILE;
}

static void update_menu_view(void) {
    int i;

    lv_label_set_text(s_lbl_title, menu_sub_pages[s_cur]->title);

    for (i = 0; i < ITEM_COUNT; i++) {
        if (i == s_cur) {
            lv_obj_set_style_bg_color(s_nav_items[i], lv_color_hex(MENU_ACCENT), 0);
            lv_obj_set_style_bg_opa(s_nav_items[i], LV_OPA_COVER, 0);
            lv_obj_set_style_text_color(s_nav_icons[i], lv_color_hex(GUI_BLACK), 0);
            lv_obj_set_style_text_opa(s_nav_icons[i], LV_OPA_COVER, 0);
        } else {
            lv_obj_set_style_bg_color(s_nav_items[i], lv_color_hex(MENU_BAR_BG), 0);
            lv_obj_set_style_bg_opa(s_nav_items[i], LV_OPA_COVER, 0);
            lv_obj_set_style_text_color(s_nav_icons[i], lv_color_hex(MENU_ACCENT), 0);
            lv_obj_set_style_text_opa(s_nav_icons[i], LV_OPA_COVER, 0);
        }
    }

    lv_obj_clean(s_preview);
    if (s_cur == 0) {
        gui_home_render(s_preview);
    } else if (s_cur == 1) {
        gui_temp_render(s_preview);
    } else if (s_cur == 2) {
        gui_weather_render(s_preview);
    } else if (s_cur == 3) {
        gui_web3_render(s_preview);
    } else if (s_cur == 4) {
        gui_config_preview(s_preview);
    }
}

static void preview_timer_cb(lv_timer_t *timer) {
    (void)timer;
    if (!s_entering && s_preview) {
        update_menu_view();
    }
}

static void card_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_KEY) {
        uint32_t key = *(uint32_t *)lv_event_get_param(e);
        if (key == LV_KEY_RIGHT) {
            s_cur = (s_cur + 1) % ITEM_COUNT;
            update_menu_view();
        } else if (key == LV_KEY_LEFT) {
            s_cur = (s_cur - 1 + ITEM_COUNT) % ITEM_COUNT;
            update_menu_view();
        } else if (key == LV_KEY_ENTER) {
            if (s_entering) return;
            s_entering = 1;
            page_mgr_push(menu_sub_pages[s_cur]);
        }
    } else if (code == LV_EVENT_CLICKED) {
        /* Encoder center-press in edit mode also fires CLICKED */
        if (s_entering) return;
        s_entering = 1;
        page_mgr_push(menu_sub_pages[s_cur]);
    }
}

static void menu_create(lv_obj_t *parent) {
    lv_obj_t *card;
    lv_obj_t *top_bar;
    lv_obj_t *nav_bar;
    lv_obj_t *label;
    int i;

    s_entering = 0;

    card = lv_obj_create(parent);
    lv_obj_set_size(card, LV_PCT(100), LV_PCT(100));
    lv_obj_set_pos(card, 0, 0);
    gui_plain_obj(card, lv_color_hex(MENU_BG));
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card,
                          LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(card, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(card, card_event_cb, LV_EVENT_ALL, NULL);
    lv_group_add_obj(lv_group_get_default(), card);

    top_bar = lv_obj_create(card);
    lv_obj_set_size(top_bar, LV_PCT(100), MENU_TOP_H);
    lv_obj_set_pos(top_bar, 0, 0);
    gui_plain_obj(top_bar, lv_color_hex(MENU_BAR_BG));
    lv_obj_set_style_border_width(top_bar, 1, 0);
    lv_obj_set_style_border_side(top_bar, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_color(top_bar, lv_color_hex(GUI_DIM), 0);

    label = gui_label(top_bar, ">_", lv_color_hex(MENU_ACCENT));
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 2, 0);

    label = gui_label(top_bar, LV_SYMBOL_BATTERY_3, lv_color_hex(MENU_ACCENT));
    lv_obj_align(label, LV_ALIGN_RIGHT_MID, -2, 0);

    s_lbl_title = lv_label_create(top_bar);
    lv_obj_set_style_text_font(s_lbl_title, GUI_FONT_SMALL, 0);
    gui_clip_label(s_lbl_title, 104);
    lv_obj_set_style_text_align(s_lbl_title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(s_lbl_title, lv_color_hex(MENU_TEXT), 0);
    lv_obj_align(s_lbl_title, LV_ALIGN_CENTER, 0, 0);

    s_preview = lv_obj_create(card);
    lv_obj_set_width(s_preview, LV_PCT(100));
    lv_obj_set_flex_grow(s_preview, 1);
    gui_plain_obj(s_preview, lv_color_hex(MENU_PREVIEW_BG));

    nav_bar = lv_obj_create(card);
    lv_obj_set_size(nav_bar, LV_PCT(100), MENU_NAV_H);
    gui_plain_obj(nav_bar, lv_color_hex(MENU_BAR_BG));
    lv_obj_set_style_border_width(nav_bar, 1, 0);
    lv_obj_set_style_border_side(nav_bar, LV_BORDER_SIDE_TOP, 0);
    lv_obj_set_style_border_color(nav_bar, lv_color_hex(GUI_DIM), 0);
    lv_obj_set_flex_flow(nav_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(nav_bar,
                          LV_FLEX_ALIGN_SPACE_EVENLY,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);

    for (i = 0; i < ITEM_COUNT; i++) {
        s_nav_items[i] = lv_obj_create(nav_bar);
        lv_obj_set_size(s_nav_items[i], 24, 20);
        gui_plain_obj(s_nav_items[i], lv_color_hex(MENU_BAR_BG));

        s_nav_icons[i] = lv_label_create(s_nav_items[i]);
        lv_label_set_text(s_nav_icons[i], get_nav_icon(i));
        lv_obj_set_style_text_font(s_nav_icons[i], GUI_FONT_BIG, 0);
        lv_obj_set_style_text_align(s_nav_icons[i], LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(s_nav_icons[i], LV_ALIGN_CENTER, 0, 0);
    }

    update_menu_view();
    if (!s_preview_timer) {
        s_preview_timer = lv_timer_create(preview_timer_cb, 1000, NULL);
    }

    /*
     * Put the group into edit mode so encoder rotation sends
     * LV_KEY_LEFT / LV_KEY_RIGHT to the focused card widget
     * instead of cycling focus between objects.
     */
    lv_group_set_editing(lv_group_get_default(), true);
}

static void menu_leave(void) {
    if (s_preview_timer) {
        lv_timer_del(s_preview_timer);
        s_preview_timer = NULL;
    }
}

const PageDef_t page_main_menu = {
    .title      = "Main Menu",
    .on_create  = menu_create,
    .on_enter   = NULL,
    .on_leave   = menu_leave,
    .on_destroy = NULL,
    .sub_pages  = menu_sub_pages,
    .sub_count  = ITEM_COUNT,
};
