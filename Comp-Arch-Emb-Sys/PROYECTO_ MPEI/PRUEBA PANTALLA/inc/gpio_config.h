#ifndef __GPIO_CONFIG_H__
#define __GPIO_CONFIG_H__

#include "stm32f446.h"

/* Pines del conector HUB75 (todos en el puerto GPIOC) */
#define PIN_R1  0    // rojo   mitad de arriba
#define PIN_G1  1    // verde  mitad de arriba
#define PIN_B1  2    // azul   mitad de arriba
#define PIN_R2  3    // rojo   mitad de abajo
#define PIN_G2  4    // verde  mitad de abajo
#define PIN_B2  5    // azul   mitad de abajo
#define PIN_OE  6    // Output Enable: 0 = LEDs encendidos, 1 = apagados
#define PIN_LAT 7    // Latch: pulso en 1 = pasar los datos enviados a los LEDs
#define PIN_CLK 8    // Clock: en cada subida el panel guarda un dato
#define PIN_A   10   // selección de fila, bit 0
#define PIN_B   11   // selección de fila, bit 1

/* Máscara de los 6 pines de color PC0..PC5: 0b0011_1111 = 0x3F */
#define MASCARA_COLORES (0x3FU << PIN_R1)

void GPIO_Config(void);
void write_pin_state(volatile GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t state);
void write_color_pins(uint8_t colores);

#endif /* __GPIO_CONFIG_H__ */
