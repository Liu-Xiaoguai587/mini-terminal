#ifndef __ST7735S_H__
#define __ST7735S_H__

#include <stdint.h>
#include "SPI.h"

#define ST7735S_MAX_HIGH    128
#define ST7735S_MAX_WIDTH   160

void ST7735s_Init(void);
void ST7735s_SetWindwos(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void ST7735s_DrawScreen(uint16_t *buf, uint16_t buf_size);
void ST7735s_SendColorsDate(uint16_t *buf, uint16_t buf_size);
void ST7735s_SendColorsDate_DMA(uint16_t *buf, uint16_t buf_size, spi_dma_done_cb_t done_cb);
void ST7735s_WriteCmd(uint8_t byte);
void ST7735s_WriteData(uint8_t byte);
void ST7735s_WriteMutiData(uint8_t* byte, uint16_t size);
void ST7735s_Clear(void);
void ST7735s_Test(void);


#endif

