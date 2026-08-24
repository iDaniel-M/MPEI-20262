#ifndef __GPIO_CONFIG_H__
#define __GPIO_CONFIG_H__
#define STM32F401xE
#include "stm32f4xx.h"
#include "stdint.h"

void GPIO_Config(void);

uint8_t read_pin_state(volatile GPIO_TypeDef *GPIOx, uint8_t pin);
void write_pin_state(volatile GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t state);

#endif // __GPIO_CONFIG_H__