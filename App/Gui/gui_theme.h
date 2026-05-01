#ifndef __GUI_THEME_H__
#define __GUI_THEME_H__

#include "gui_page.h"

#define GUI_AMBER      0xFF9D00
#define GUI_BLACK      0x000000
#define GUI_DARK       0x080604
#define GUI_DIM        0x5C3A00
#define GUI_PANEL      0x120C05
#define GUI_TOP_H      14
#define GUI_NAV_H      28
#define GUI_FONT_SMALL (&lv_font_montserrat_10)
#define GUI_FONT       (&lv_font_montserrat_12)
#define GUI_FONT_BIG   (&lv_font_montserrat_14)
#define GUI_FONT_HUGE  (&lv_font_montserrat_32)

static inline void gui_plain_obj(lv_obj_t *obj, lv_color_t bg) {
    lv_obj_set_style_radius(obj, 0, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_set_style_bg_color(obj, bg, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}

static inline lv_obj_t *gui_label(lv_obj_t *parent,
                                  const char *text,
                                  lv_color_t color) {
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, GUI_FONT, 0);
    lv_obj_set_style_text_color(label, color, 0);
    return label;
}

static inline void gui_clip_label(lv_obj_t *label, lv_coord_t width) {
    lv_obj_set_width(label, width);
    lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
}

static inline lv_obj_t *gui_panel(lv_obj_t *parent,
                                  lv_coord_t w,
                                  lv_coord_t h) {
    lv_obj_t *panel = lv_obj_create(parent);
    lv_obj_set_size(panel, w, h);
    lv_obj_set_style_radius(panel, 0, 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(GUI_AMBER), 0);
    lv_obj_set_style_border_opa(panel, LV_OPA_70, 0);
    lv_obj_set_style_pad_all(panel, 2, 0);
    lv_obj_set_style_bg_color(panel, lv_color_hex(GUI_BLACK), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    return panel;
}

static inline lv_obj_t *gui_page_content(lv_obj_t *parent,
                                         const char *title) {
    lv_obj_t *root;
    lv_obj_t *top;
    lv_obj_t *content;
    lv_obj_t *left;
    lv_obj_t *right;
    lv_obj_t *caption;

    root = lv_obj_create(parent);
    lv_obj_set_size(root, LV_PCT(100), LV_PCT(100));
    gui_plain_obj(root, lv_color_hex(GUI_BLACK));
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);

    top = lv_obj_create(root);
    lv_obj_set_size(top, LV_PCT(100), GUI_TOP_H);
    gui_plain_obj(top, lv_color_hex(GUI_BLACK));
    lv_obj_set_style_border_width(top, 1, 0);
    lv_obj_set_style_border_side(top, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_color(top, lv_color_hex(GUI_DIM), 0);

    left = gui_label(top, ">_", lv_color_hex(GUI_AMBER));
    lv_obj_set_style_text_font(left, GUI_FONT_SMALL, 0);
    lv_obj_align(left, LV_ALIGN_LEFT_MID, 2, 0);

    caption = gui_label(top, title, lv_color_hex(GUI_AMBER));
    lv_obj_set_style_text_font(caption, GUI_FONT_SMALL, 0);
    gui_clip_label(caption, 104);
    lv_obj_set_style_text_align(caption, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(caption, LV_ALIGN_CENTER, 0, 0);

    right = gui_label(top, LV_SYMBOL_BATTERY_3, lv_color_hex(GUI_AMBER));
    lv_obj_set_style_text_font(right, GUI_FONT_SMALL, 0);
    lv_obj_align(right, LV_ALIGN_RIGHT_MID, -2, 0);

    content = lv_obj_create(root);
    lv_obj_set_width(content, LV_PCT(100));
    lv_obj_set_flex_grow(content, 1);
    gui_plain_obj(content, lv_color_hex(GUI_BLACK));
    lv_obj_set_style_pad_all(content, 4, 0);

    return content;
}

static inline lv_obj_t *gui_status_cursor(lv_obj_t *parent, const char *text) {
    lv_obj_t *status;
    lv_obj_t *cursor;

    status = gui_label(parent, text, lv_color_hex(GUI_DIM));
    lv_obj_set_style_text_font(status, GUI_FONT_SMALL, 0);
    lv_obj_align(status, LV_ALIGN_BOTTOM_LEFT, 2, -1);

    cursor = lv_obj_create(parent);
    lv_obj_set_size(cursor, 4, 9);
    gui_plain_obj(cursor, lv_color_hex(GUI_AMBER));
    lv_obj_align(cursor, LV_ALIGN_BOTTOM_RIGHT, -2, -2);

    return status;
}

#endif /* __GUI_THEME_H__ */
