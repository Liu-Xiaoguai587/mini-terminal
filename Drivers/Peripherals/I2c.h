#ifndef __I2C_H__
#define __I2C_H__

#include "stm32f4xx.h"

void    IIC_Init(void);
uint8_t IIC_Write    (uint8_t dev_addr, uint8_t reg_addr, uint8_t data);
uint8_t IIC_Read     (uint8_t dev_addr, uint8_t reg_addr, uint8_t *data);
uint8_t IIC_Write_Buf(uint8_t dev_addr, uint8_t reg_addr, uint8_t *buf, uint16_t len);
uint8_t IIC_Read_Buf (uint8_t dev_addr, uint8_t reg_addr, uint8_t *buf, uint16_t len);
uint8_t IIC_Write_Raw(uint8_t dev_addr, uint8_t *buf, uint16_t len);
uint8_t IIC_Read_Raw (uint8_t dev_addr, uint8_t *buf, uint16_t len);

#endif // __I2C_H__

