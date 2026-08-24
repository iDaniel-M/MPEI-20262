#ifndef __GPIO_CONFIG_H__
#define __GPIO_CONFIG_H__
#include "TM4C123GH6PM.h"
#include "stdint.h"

#define RGB_LED_PIN1 1 // PF1
#define RGB_LED_PIN2 2 // PF2
#define RGB_LED_PIN3 3 // PF3

void GPIO_Config(void);

uint8_t read_pin_state(volatile GPIO_TypeDef *GPIOx, uint8_t pin);
void write_pin_state(volatile GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t state);

#endif // __GPIO_CONFIG_H__