/* Simulator stub — replaces STM32 HAL stm32f4xx.h */
#ifndef STM32F4XX_H
#define STM32F4XX_H
#include <stdint.h>

/* Minimal GPIO types so Peripherals headers compile */
typedef struct { uint32_t IDR; } GPIO_TypeDef;

#define GPIOB  ((GPIO_TypeDef *)0)
#define GPIO_Pin_0  ((uint16_t)0x0001)
#define GPIO_Pin_1  ((uint16_t)0x0002)

/* Always return 1 (not pressed) in simulator */
static inline uint8_t GPIO_ReadInputDataBit(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
    (void)GPIOx; (void)GPIO_Pin;
    return 1;
}

#endif /* STM32F4XX_H */
