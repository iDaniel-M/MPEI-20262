#include "gpio_config.h"

/* Configura un pin de GPIOC como salida rápida.
 * MODER y OSPEEDR usan 2 bits por pin: los del pin n están en las posiciones 2n y 2n+1. */
static void configurar_salida(uint8_t pin)
{
    GPIOC->MODER &= ~(3U << (2 * pin));    // borrar los 2 bits del pin (00)
    GPIOC->MODER |=  (1U << (2 * pin));    // 01 = salida de propósito general

    GPIOC->OSPEEDR &= ~(3U << (2 * pin));  // borrar los 2 bits del pin
    GPIOC->OSPEEDR |=  (2U << (2 * pin));  // 10 = velocidad alta (flancos limpios en CLK)
}

void GPIO_Config(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;   // encender el reloj de GPIOC (bit 2)

    // dos lecturas de espera después de encender el reloj (errata del STM32F4)
    volatile uint32_t dummy;
    dummy = RCC->AHB1ENR;
    dummy = RCC->AHB1ENR;
    (void)dummy;

    // Antes de volver salidas los pines, dejar OE en 1 (LEDs apagados) y el resto en 0,
    // para que el panel no muestre basura al arrancar.
    GPIOC->ODR = (1U << PIN_OE);

    configurar_salida(PIN_R1);
    configurar_salida(PIN_G1);
    configurar_salida(PIN_B1);
    configurar_salida(PIN_R2);
    configurar_salida(PIN_G2);
    configurar_salida(PIN_B2);
    configurar_salida(PIN_OE);
    configurar_salida(PIN_LAT);
    configurar_salida(PIN_CLK);
    configurar_salida(PIN_A);
    configurar_salida(PIN_B);
}

/* Pone un pin en 1 o en 0 sin tocar los demás pines del puerto (igual que en el curso). */
void write_pin_state(volatile GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t state)
{
    if (state) {
        GPIOx->ODR |= (1U << pin);     // OR con 1 en ese bit: lo pone en 1
    } else {
        GPIOx->ODR &= ~(1U << pin);    // AND con 0 en ese bit: lo pone en 0
    }
}

/* Escribe los 6 pines de color a la vez.
 * colores: bit0 R1, bit1 G1, bit2 B1, bit3 R2, bit4 G2, bit5 B2 (igual que PC0..PC5). */
void write_color_pins(uint8_t colores)
{
    uint32_t valor = GPIOC->ODR;
    valor &= ~MASCARA_COLORES;                        // 1) borrar los 6 bits de color
    valor |= ((uint32_t)colores << PIN_R1) & MASCARA_COLORES; // 2) poner los nuevos
    GPIOC->ODR = valor;                               // 3) escribir todo de una vez
}
