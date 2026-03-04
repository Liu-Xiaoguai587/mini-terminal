
#include <stdint.h>
#include "ST7735s.h"



#ifdef AUTO_UPDATE_DISABLE
#define UPDATE 
#else
#define UPDATE GL_Update()
#endif

#define __DRAWPIXEL(x, y, color)    frame_buf[y][x] = color;
#define __COLOR(c)                  ( *((uint8_t *)&c + 1) + ((uint8_t)c << 8) )
#define __ABS(x)                    ((x) > 0 ? x : -(x))
#define __MAX(x, y)                 ( (x) > (y) ? (x) : (y) )
#define __MIN(x, y)                 ( (x) > (y) ? (y) : (x) )
#define __SWAP8B(x, y)              do { uint8_t t = x; x = y; y = t; } while(0)


uint16_t frame_buf[ST7735S_MAX_HIGH][ST7735S_MAX_WIDTH] = {0x0000};

void GL_Update(void) {
    ST7735s_DrawScreen((uint16_t *)frame_buf, ST7735S_MAX_HIGH * ST7735S_MAX_WIDTH);
}

void GL_Clear(uint16_t color) {
    for(uint8_t i = 0; i < ST7735S_MAX_HIGH; i++)
        for(uint8_t j = 0; j < ST7735S_MAX_WIDTH; j++) {
            __DRAWPIXEL(j, i, __COLOR(color));
        }

    // GL_Update();
    UPDATE;
}

void GL_DrawPixel(uint8_t x, uint8_t y, uint16_t color) {
    frame_buf[y][x] = __COLOR(color);
    // frame_buf[x][y] = __COLOR(color);

    // GL_Update();
    UPDATE;
}

void GL_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color) {
    int16_t dx = __ABS(x2 - x1);
    int16_t dy = __ABS(y2 - y1);
    int step_x = x1 < x2 ? 1 : -1;
    int step_y = y1 < y2 ? 1 : -1;

    int16_t err = dx - dy;

    while (1) {
        __DRAWPIXEL(x1, y1, __COLOR(color));

        if (x1 == x2 && y1 == y2)
            break;

        int16_t err2 = err * 2;

        if (err2 > -dy) {
            err -= dy;
            x1 += step_x;
        }

        if (err2 < dx) {
            err += dx;
            y1 += step_y;
        }
    }
    
    // GL_Update();
    UPDATE;
}

void GL_DrawRectange(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color) {
    if(x1 > x2) __SWAP8B(x1, x2);
    if(y1 > y2) __SWAP8B(y1, y2);

    for (uint8_t i = y1; i < (y2 - y1); i++) {
        for (uint8_t j = x1; j < (x2 - x1); j++) { 
            __DRAWPIXEL(j, i, __COLOR(color));
        }
    }

    // GL_Update();
    UPDATE;
}

void GL_DrawCircle(uint8_t x1, uint8_t y1, uint8_t r, uint16_t color) {
    int16_t x = 0;
    int16_t y = r;
    int16_t d = 3 - 2 * r; // 初始决策变量

    while (x <= y) {
        // 利用 8 块对称性画点
        __DRAWPIXEL(x1 + x, y1 + y, __COLOR(color)); __DRAWPIXEL(x1 - x, y1 + y, __COLOR(color));
        __DRAWPIXEL(x1 + x, y1 - y, __COLOR(color)); __DRAWPIXEL(x1 - x, y1 - y, __COLOR(color));
        __DRAWPIXEL(x1 + y, y1 + x, __COLOR(color)); __DRAWPIXEL(x1 - y, y1 + x, __COLOR(color));
        __DRAWPIXEL(x1 + y, y1 - x, __COLOR(color)); __DRAWPIXEL(x1 - y, y1 - x, __COLOR(color));

        if (d < 1) {
            // 下一个点在圆内，y 不动
            d = d + 4 * x + 6;
        } else {
            // 下一个点在圆外，y 减 1 向内靠拢
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
    // GL_Update();
    UPDATE;
}

static void _GL_DrawHLine(uint8_t x1, uint8_t x2, uint8_t y, uint8_t color) {
    for (uint16_t i = x1; i <= x2; i++) {
        __DRAWPIXEL(i, y, color);
    }
}

void GL_DrawFillCircle(uint8_t x0, uint8_t y0, uint8_t r, uint16_t color) {
    int16_t x = 0;
    int16_t y = r;
    int16_t d = 3 - 2 * r;

    while (x <= y) {
        // 将对称的 8 个点连成 4 条横线
        _GL_DrawHLine(x0 - x, x0 + x, y0 + y, __COLOR(color));
        _GL_DrawHLine(x0 - x, x0 + x, y0 - y, __COLOR(color));
        _GL_DrawHLine(x0 - y, x0 + y, y0 + x, __COLOR(color));
        _GL_DrawHLine(x0 - y, x0 + y, y0 - x, __COLOR(color));

        if (d < 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
    UPDATE;
}
