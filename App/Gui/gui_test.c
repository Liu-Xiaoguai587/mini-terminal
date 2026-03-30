#include "gui_page.h"
#include "EC11.h"

/* ══════════════════════════════════════════════════════════
 *  Test Page 1 — Slider demo
 * ══════════════════════════════════════════════════════════ */
static void test_create(lv_obj_t *parent) {
    lv_obj_t *slider = lv_slider_create(parent);
    lv_slider_set_range(slider, 0, 10);
    lv_slider_set_value(slider, 5, LV_ANIM_OFF);
    lv_obj_align(slider, LV_ALIGN_CENTER, 0, -20);
    lv_group_add_obj(lv_group_get_default(), slider);

    lv_obj_t *slider2 = lv_slider_create(parent);
    lv_slider_set_range(slider2, 0, 10);
    lv_slider_set_value(slider2, 5, LV_ANIM_OFF);
    lv_obj_align(slider2, LV_ALIGN_CENTER, 0, 20);
    lv_group_add_obj(lv_group_get_default(), slider2);
}

const PageDef_t page_test = {
    .title      = "Slider Test",
    .on_create  = test_create,
    .on_enter   = NULL,
    .on_leave   = NULL,
    .on_destroy = NULL,
    .sub_pages  = NULL,
    .sub_count  = 0,
};

/* ══════════════════════════════════════════════════════════
 *  Test Page 2 — Button GPIO monitor
 * ══════════════════════════════════════════════════════════ */
static lv_timer_t *btn_test_timer = NULL;

static void btn_test_poll(lv_timer_t *t) {
    lv_obj_t *lb = (lv_obj_t *)t->user_data;
    lv_label_set_text_fmt(lb,
        "RAW GPIO:\n"
        " PB0(SELECT) = %d\n"
        " PB1(EXIT)   = %d\n"
        "(0 = pressed)",
        GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0),
        GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1));
}

static void btn_test_create(lv_obj_t *parent) {
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, "Button Test");
    lv_obj_center(label);
    btn_test_timer = lv_timer_create(btn_test_poll, 100, label);
}

static void btn_test_leave(void) {
    if (btn_test_timer) {
        lv_timer_del(btn_test_timer);
        btn_test_timer = NULL;
    }
}

const PageDef_t page_btn_test = {
    .title      = "Button Test",
    .on_create  = btn_test_create,
    .on_enter   = NULL,
    .on_leave   = btn_test_leave,
    .on_destroy = NULL,
    .sub_pages  = NULL,
    .sub_count  = 0,
};
