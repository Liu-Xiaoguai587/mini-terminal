#ifndef __GL_H__
#define __GL_H__

#include <stdint.h>

void GL_Update(void);
void GL_Clear(uint16_t color);
void GL_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color);
void GL_DrawRectange(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t );
void GL_DrawCircle(uint8_t x1, uint8_t y1, uint8_t r, uint16_t color);
void GL_DrawFillCircle(uint8_t x0, uint8_t y0, uint8_t r, uint16_t color);
void GL_DrawPixel(uint8_t x, uint8_t y, uint16_t color);

#endif
