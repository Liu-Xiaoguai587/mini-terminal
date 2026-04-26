#include "lvgl.h"
#include <SDL2/SDL.h>
#include <stdint.h>

/*
 * Keyboard mapping:
 *   UP    arrow  → encoder rotate counter-clockwise (prev item)
 *   DOWN  arrow  → encoder rotate clockwise (next item)
 *   ENTER/SPACE  → encoder center-click (select / enter edit mode)
 *   ESC/BACKSPACE → handled in main loop → page_mgr_pop()
 */

static int16_t            s_enc_diff = 0;
static lv_indev_state_t   s_btn_state = LV_INDEV_STATE_RELEASED;

void sim_indev_key_down(SDL_Keycode sym) {
    switch (sym) {
        case SDLK_UP:     s_enc_diff--; break;
        case SDLK_DOWN:   s_enc_diff++; break;
        case SDLK_RETURN:
        case SDLK_SPACE:  s_btn_state = LV_INDEV_STATE_PRESSED;  break;
        default: break;
    }
}

void sim_indev_key_up(SDL_Keycode sym) {
    switch (sym) {
        case SDLK_RETURN:
        case SDLK_SPACE:  s_btn_state = LV_INDEV_STATE_RELEASED; break;
        default: break;
    }
}

static void encoder_read_cb(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    (void)drv;
    data->enc_diff = s_enc_diff;
    data->state    = s_btn_state;
    s_enc_diff = 0;
}

void sim_indev_init(void) {
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type    = LV_INDEV_TYPE_ENCODER;
    indev_drv.read_cb = encoder_read_cb;
    lv_indev_t *indev = lv_indev_drv_register(&indev_drv);

    lv_group_t *g = lv_group_create();
    lv_group_set_default(g);
    lv_indev_set_group(indev, g);
}
