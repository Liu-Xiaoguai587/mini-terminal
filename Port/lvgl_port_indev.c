#include "EC11.h"
#include "lvgl.h"

static lv_indev_t *encoder_indev;
static lv_indev_t *keypad_indev;

void EC11_read_cb(lv_indev_drv_t * indev_drv, lv_indev_data_t * data) {
    data->enc_diff = EC11_Get_Count();
    data->state = (Button_Get_State() & BTN_SELECT) ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

void keypad_read_cb(lv_indev_drv_t * indev_drv, lv_indev_data_t * data) {
    data->key   = LV_KEY_ESC;
    data->state = (Button_Get_State() & BTN_EXIT) ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

void lv_port_indev_init(void) {
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type    = LV_INDEV_TYPE_ENCODER;
    indev_drv.read_cb = EC11_read_cb;
    encoder_indev = lv_indev_drv_register(&indev_drv);

    static lv_indev_drv_t keypad_drv;
    lv_indev_drv_init(&keypad_drv);
    keypad_drv.type    = LV_INDEV_TYPE_KEYPAD;
    keypad_drv.read_cb = keypad_read_cb;
    keypad_indev = lv_indev_drv_register(&keypad_drv);

    lv_group_t *g = lv_group_create();
    lv_group_set_default(g);
    lv_indev_set_group(encoder_indev, g);
    lv_indev_set_group(keypad_indev, g);
}
