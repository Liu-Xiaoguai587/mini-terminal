#ifndef __SPI_H__
#define __SPI_H__

#include <stdint.h>

typedef void (*spi_dma_done_cb_t)(void);

void Abstract_SPI_Init(void);
void ABstract_SPI_DMA_Init(void);
void Abstract_SPI_DMA_Send(uint8_t *buf, uint16_t buf_size);
void Abstract_SPI_DMA_Send_IT(uint8_t *buf, uint16_t buf_size, spi_dma_done_cb_t done_cb);
uint8_t Abstract_SPI_DMA_Busy(void);
void Abstract_SPI_SendByte(uint8_t byte);
void Abstract_SPI_SendMutiByte(uint8_t* bytes, uint16_t size);

#endif
