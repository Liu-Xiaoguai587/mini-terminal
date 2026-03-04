#include "lvgl.h"
#include "src/widgets/lv_slider.h"

void gui_test(void) {
    lv_obj_t *slider = lv_slider_create(lv_scr_act());
    lv_slider_set_range(slider, 0, 10);
    lv_slider_set_value(slider, 5, LV_ANIM_OFF);
    lv_obj_center(slider);
}
