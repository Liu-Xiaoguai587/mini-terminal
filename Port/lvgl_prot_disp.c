#include "lvgl.h"
#include "ST7735s.h"

static lv_disp_drv_t *s_pending_disp_drv = 0;

static void lv_dma_flush_done(void) {
    lv_disp_drv_t *drv = s_pending_disp_drv;
    s_pending_disp_drv = 0;
    if (drv) lv_disp_flush_ready(drv);
}

void  lv_flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p){
    ST7735s_SetWindwos(area->x1, area->y1, area->x2, area->y2);

    uint16_t size = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1);

    s_pending_disp_drv = disp_drv;
    ST7735s_SendColorsDate_DMA((uint16_t *)color_p, size, lv_dma_flush_done);
}

void lv_link2_st7735s(void) {
    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t buf1[160 * 16];
    static lv_color_t buf2[160 * 16];
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, 160 * 16);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);

    disp_drv.hor_res = 160;
    disp_drv.ver_res = 128;
    disp_drv.flush_cb = lv_flush_cb;
    disp_drv.draw_buf = &draw_buf;

    lv_disp_drv_register(&disp_drv);
}
