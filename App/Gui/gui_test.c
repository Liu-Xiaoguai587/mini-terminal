#include "lvgl.h"
#include "src/widgets/lv_slider.h"
#include "EC11.h"

static lv_timer_t *btn_test_timer = NULL;

static void btn_test_cleanup(lv_event_t *e) {
    if (btn_test_timer) { lv_timer_del(btn_test_timer); btn_test_timer = NULL; }
}

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

void gui_test2(void) {
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Button Test");
    lv_obj_center(label);
    lv_obj_add_event_cb(label, btn_test_cleanup, LV_EVENT_DELETE, NULL);
    btn_test_timer = lv_timer_create(btn_test_poll, 100, label);
}

void gui_test(void) {
    lv_obj_t *slider = lv_slider_create(lv_scr_act());
    lv_slider_set_range(slider, 0, 10);
    lv_slider_set_value(slider, 5, LV_ANIM_OFF);
    lv_obj_align(slider, LV_ALIGN_CENTER, 0, -20);
    lv_group_add_obj(lv_group_get_default(), slider);

    lv_obj_t *slider2 = lv_slider_create(lv_scr_act());
    lv_slider_set_range(slider2, 0, 10);
    lv_slider_set_value(slider2, 5, LV_ANIM_OFF);
    lv_obj_align(slider2, LV_ALIGN_CENTER, 0, 20);
    lv_group_add_obj(lv_group_get_default(), slider2);
}
