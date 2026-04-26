#include "lvgl.h"
#include <SDL2/SDL.h>

#define SIM_W     160
#define SIM_H     128
#define SIM_SCALE 1   /* window = 160 x 128 */

static SDL_Window   *sim_window;
static SDL_Renderer *sim_renderer;
static SDL_Texture  *sim_texture;

/* Full-frame ARGB8888 buffer — updated each flush, re-uploaded to texture */
static uint32_t fb[SIM_W * SIM_H];

static void sim_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p) {
    int32_t x, y;
    for (y = area->y1; y <= area->y2; y++) {
        for (x = area->x1; x <= area->x2; x++) {
            uint16_t px = color_p->full;
            /* LV_COLOR_16_SWAP=1: bytes are swapped for the SPI display.
             * Swap back to get normal RGB565 for the PC renderer. */
            px = (uint16_t)((px >> 8) | (px << 8));

            uint8_t r = (uint8_t)(((px >> 11) & 0x1F) << 3);
            uint8_t g = (uint8_t)(((px >>  5) & 0x3F) << 2);
            uint8_t b = (uint8_t)((px & 0x1F) << 3);

            fb[y * SIM_W + x] = (0xFFu << 24) | ((uint32_t)r << 16) |
                                 ((uint32_t)g << 8) | b;
            color_p++;
        }
    }

    SDL_UpdateTexture(sim_texture, NULL, fb, SIM_W * (int)sizeof(uint32_t));
    SDL_RenderClear(sim_renderer);
    SDL_RenderCopy(sim_renderer, sim_texture, NULL, NULL);
    SDL_RenderPresent(sim_renderer);

    lv_disp_flush_ready(drv);
}

void sim_disp_init(void) {
    sim_window = SDL_CreateWindow(
        "mini-terminal sim",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SIM_W * SIM_SCALE, SIM_H * SIM_SCALE,
        SDL_WINDOW_SHOWN
    );

    sim_renderer = SDL_CreateRenderer(sim_window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    sim_texture = SDL_CreateTexture(
        sim_renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        SIM_W, SIM_H
    );

    /* Register LVGL display driver */
    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t buf[SIM_W * 16];
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, SIM_W * 16);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res  = SIM_W;
    disp_drv.ver_res  = SIM_H;
    disp_drv.flush_cb = sim_flush_cb;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
}
