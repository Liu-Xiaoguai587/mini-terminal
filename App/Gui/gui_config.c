#include "gui_page.h"
#include "gui_theme.h"

static void config_row(lv_obj_t *parent,
                       lv_align_t align,
                       lv_coord_t x,
                       lv_coord_t y,
                       const char *name,
                       const char *value,
                       uint8_t active) {
    lv_obj_t *row;
    lv_obj_t *left;
    lv_obj_t *right;

    row = lv_obj_create(parent);
    lv_obj_set_size(row, 66, 18);
    lv_obj_align(row, align, x, y);
    gui_plain_obj(row, active ? lv_color_hex(GUI_AMBER)
                              : lv_color_hex(GUI_BLACK));
    lv_obj_set_style_border_width(row, 1, 0);
    lv_obj_set_style_border_color(row, lv_color_hex(GUI_AMBER), 0);

    left = gui_label(row, name, active ? lv_color_hex(GUI_BLACK)
                                      : lv_color_hex(GUI_AMBER));
    gui_clip_label(left, 42);
    lv_obj_align(left, LV_ALIGN_LEFT_MID, 3, 0);

    right = gui_label(row, value, active ? lv_color_hex(GUI_BLACK)
                                        : lv_color_hex(GUI_AMBER));
    lv_obj_set_style_text_font(right, GUI_FONT_SMALL, 0);
    gui_clip_label(right, 20);
    lv_obj_set_style_text_align(right, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_align(right, LV_ALIGN_RIGHT_MID, -3, 0);
}

static void config_render_options(lv_obj_t *parent,
                                  lv_coord_t first_y,
                                  lv_coord_t second_y) {
    config_row(parent, LV_ALIGN_TOP_LEFT, 5, first_y, ">Wifi", "OK", 1);
    config_row(parent, LV_ALIGN_TOP_RIGHT, -5, first_y, "Bright", "80", 0);
    config_row(parent, LV_ALIGN_TOP_LEFT, 5, second_y, "Time", "", 0);
    config_row(parent, LV_ALIGN_TOP_RIGHT, -5, second_y, "Reset", "", 0);
}

void gui_config_preview(lv_obj_t *parent) {
    lv_obj_t *title;

    gui_plain_obj(parent, lv_color_hex(GUI_BLACK));
    lv_obj_set_style_pad_all(parent, 0, 0);

    title = gui_label(parent, "> SELECT_OPT:", lv_color_hex(GUI_DIM));
    lv_obj_set_style_text_font(title, GUI_FONT_SMALL, 0);
    gui_clip_label(title, 120);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 4, 2);

    config_render_options(parent, 18, 40);
}

void gui_config_render(lv_obj_t *parent) {
    lv_obj_t *title;
    lv_obj_t *foot;

    gui_plain_obj(parent, lv_color_hex(GUI_BLACK));
    lv_obj_set_style_pad_all(parent, 4, 0);

    title = gui_label(parent, "> SELECT_OPT:", lv_color_hex(GUI_DIM));
    lv_obj_set_style_text_font(title, GUI_FONT_SMALL, 0);
    gui_clip_label(title, 120);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 4, 2);

    config_render_options(parent, 18, 40);

    foot = gui_label(parent, "SYS.MEM: 640K OK", lv_color_hex(GUI_DIM));
    lv_obj_set_style_text_font(foot, GUI_FONT_SMALL, 0);
    gui_clip_label(foot, 118);
    lv_obj_set_style_text_align(foot, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(foot, LV_ALIGN_BOTTOM_MID, 0, -1);
}

static void config_create(lv_obj_t *parent) {
    lv_obj_t *content = gui_page_content(parent, "[CONFIG_MENU]");
    gui_config_render(content);
}

const PageDef_t page_config = {
    .title      = "[CONFIG_MENU]",
    .on_create  = config_create,
    .on_enter   = NULL,
    .on_leave   = NULL,
    .on_destroy = NULL,
    .sub_pages  = NULL,
    .sub_count  = 0,
};
