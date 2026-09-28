#ifndef __GPIO_CONFIG_H__
#define __GPIO_CONFIG_H__
#define STM32F401xE
#include "stm32f4xx.h"
#include "stdint.h"
#include "system_config.h"

typedef struct {
    uint32_t Mode;
    uint32_t Pin;
    uint32_t Pull;
    uint32_t Speed;
    uint32_t Alternate;

} GPIO_InitTypeDef;

void GPIO_clock_enable(volatile GPIO_TypeDef *GPIOx);

void GPIO_Config(volatile GPIO_TypeDef *GPIOx, GPIO_InitTypeDef Config);

uint32_t read_pin_state(volatile GPIO_TypeDef *GPIOx, uint32_t pin);
void write_pin_state(volatile GPIO_TypeDef *GPIOx, uint32_t pin, uint32_t state);
void toggle_pin_state(volatile GPIO_TypeDef *GPIOx, uint32_t pin);

#endif // __GPIO_CONFIG_H__