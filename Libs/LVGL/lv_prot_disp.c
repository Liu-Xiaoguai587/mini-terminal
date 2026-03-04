#include "lvgl.h"
#include "ST7735s.h"
#include "src/hal/lv_hal_disp.h"
#include <stdint.h>


void  lv_flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p){
    ST7735s_SetWindwos(area->x1, area->y1, area->x2, area->y2);

    uint16_t size = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1);

    ST7735s_SendColorsDate((uint16_t *)color_p, size);

    lv_disp_flush_ready(disp_drv);
}

void lv_link2_st7735s(void) {
    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t buf1[128 * 160];
    lv_disp_draw_buf_init(&draw_buf, buf1, NULL, 128 * 160);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);

    disp_drv.hor_res = 160;
    disp_drv.ver_res = 128;
    disp_drv.flush_cb = lv_flush_cb;
    disp_drv.draw_buf = &draw_buf;

    lv_disp_drv_register(&disp_drv);
}


// void lv_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
//     uint8_t x1 = area->x1;
//     uint8_t y1 = area->y1;
//     uint8_t x2 = area->x2;
//     uint8_t y2 = area->y2;
//     ST7735s_SetWindwos(x1, y1, x2, y2);
//     
//     uint16_t pixel_count = (x2 - x1 + 1) * (y2 - y1 + 1);
//     ST7735s_SendColorsDate((uint16_t *)px_map, pixel_count);
//
//     lv_display_flush_ready(disp);
// }
//
//
// void lv_ST7735s_link2_lvgl_init() {
//     static uint8_t fb[128 * 160 * 2];
//
//     lv_display_t *disp = lv_display_create(128, 160);
//     lv_display_set_buffers(disp, fb, NULL, sizeof(fb), LV_DISPLAY_RENDER_MODE_DIRECT);
//     lv_display_set_flush_cb(disp, lv_flush_cb);
//     lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
// }
//

