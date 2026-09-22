#include "gpio_config.h"

void GPIO_Config(void)
{
    // 1. Habilitar relojes para GPIOA, GPIOB y GPIOC en el bus AHB1
    RCC->AHB1ENR |= (1 << 0); // GPIOA
    RCC->AHB1ENR |= (1 << 1); // GPIOB
    RCC->AHB1ENR |= (1 << 2); // GPIOC

    volatile unsigned int dummy = RCC->AHB1ENR;
    (void)dummy;

    // 2. ENTRADAS: Teclado matricial en GPIOB (Sin modificar)
    uint8_t row_pins[] = {1, 0, 12, 15};
    for(int i = 0; i < 4; i++) {
        uint8_t p = row_pins[i];
        GPIOB->MODER &= ~(3 << (p * 2));
        GPIOB->MODER |=  (1 << (p * 2));
    }

    uint8_t col_pins[] = {5, 6, 7, 8};
    for(int i = 0; i < 4; i++) {
        uint8_t p = col_pins[i];
        GPIOB->MODER &= ~(3 << (p * 2));
        GPIOB->PUPDR &= ~(3 << (p * 2));
        GPIOB->PUPDR |=  (1 << (p * 2));
    }

    // 3. SALIDAS: Añadido PA11 en el arreglo de GPIOA
    uint8_t gpioa_outputs[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 15};
    for(int i = 0; i < 14; i++) {
        uint8_t p = gpioa_outputs[i];
        GPIOA->MODER &= ~(3 << (p * 2));
        GPIOA->MODER |=  (1 << (p * 2)); // Salida
    }

    // GPIOB: B9
    GPIOB->MODER &= ~(3 << (9 * 2));
    GPIOB->MODER |=  (1 << (9 * 2));

    // GPIOC: C13, C14, C15
    uint8_t gpioc_outputs[] = {13, 14, 15};
    for(int i = 0; i < 3; i++) {
        uint8_t p = gpioc_outputs[i];
        GPIOC->MODER &= ~(3 << (p * 2));
        GPIOC->MODER |=  (1 << (p * 2));
    }
}

uint8_t read_pin_state(volatile GPIO_TypeDef *GPIOx, uint8_t pin) {
    return (GPIOx->IDR & (1 << pin)) ? 1 : 0;
}

void write_pin_state(volatile GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t state) {
    if(state) {
        GPIOx->ODR |= (1 << pin);
    } else {
        GPIOx->ODR &= ~(1 << pin);
    }
}