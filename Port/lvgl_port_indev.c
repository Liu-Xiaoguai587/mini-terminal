#include "EC11.h"
#include "lvgl.h"

static lv_indev_t *encoder_indev;

static void EC11_read_cb(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
    data->enc_diff = EC11_Get_Count();
    data->state = (Button_Get_State() & BTN_SELECT)
                      ? LV_INDEV_STATE_PRESSED
                      : LV_INDEV_STATE_RELEASED;
}

void lv_port_indev_init(void) {
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type    = LV_INDEV_TYPE_ENCODER;
    indev_drv.read_cb = EC11_read_cb;
    encoder_indev = lv_indev_drv_register(&indev_drv);

    lv_group_t *g = lv_group_create();
    lv_group_set_default(g);
    lv_indev_set_group(encoder_indev, g);
}
