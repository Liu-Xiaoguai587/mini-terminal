#include "lvgl.h"
#include "gui_page.h"
#include <SDL2/SDL.h>
#include <stdio.h>

extern const PageDef_t page_main_menu;

void sim_disp_init(void);
void sim_indev_init(void);
void sim_indev_key_down(SDL_Keycode sym);
void sim_indev_key_up(SDL_Keycode sym);

int main(void) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    lv_init();
    sim_disp_init();
    sim_indev_init();

    page_mgr_init(&page_main_menu);

    uint32_t last_tick = SDL_GetTicks();
    int running = 1;

    printf("Controls: UP/DOWN = encoder, ENTER/SPACE = select, ESC/BACKSPACE = back, Q = quit\n");

    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
            } else if (e.type == SDL_KEYDOWN) {
                SDL_Keycode sym = e.key.keysym.sym;
                if (sym == SDLK_q) {
                    running = 0;
                } else if (sym == SDLK_ESCAPE || sym == SDLK_BACKSPACE) {
                    page_mgr_pop();
                } else {
                    sim_indev_key_down(sym);
                }
            } else if (e.type == SDL_KEYUP) {
                sim_indev_key_up(e.key.keysym.sym);
            }
        }

        uint32_t now = SDL_GetTicks();
        lv_tick_inc(now - last_tick);
        last_tick = now;

        lv_timer_handler();
        SDL_Delay(5);
    }

    SDL_Quit();
    return 0;
}
