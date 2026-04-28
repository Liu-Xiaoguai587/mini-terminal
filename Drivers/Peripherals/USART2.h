#ifndef __USART2_H__
#define __USART2_H__

#include "stm32f4xx.h"

void     USART2_Init(uint32_t baudrate);
void     USART2_SendByte(uint8_t byte);
void     USART2_SendStr(const char *str);
void     USART2_SendBuf(const uint8_t *buf, uint16_t len);
uint16_t USART2_ReadLine(char *buf, uint16_t max_len, uint32_t timeout_ms);
uint16_t USART2_ReadBytes(uint8_t *buf, uint16_t count, uint32_t timeout_ms);
void     USART2_FlushRX(void);

#endif /* __USART2_H__ */
