#ifndef __SPI_H__
#define __SPI_H__

#include <stdint.h>

void Abstract_SPI_Init(void);
void ABstract_SPI_DMA_Init(void);
void Abstract_SPI_DMA_Send(uint8_t *buf, uint16_t buf_size);
void Abstract_SPI_SendByte(uint8_t byte);
void Abstract_SPI_SendMutiByte(uint8_t* bytes, uint16_t size);

#endif
