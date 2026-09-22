#ifndef __CONFIGURACION_GPIO_H__
#define __CONFIGURACION_GPIO_H__

#include "stm32f401.h" // Importamos mapa de registros directo al bare-metal del ARM Cortex-M4
#include <stdint.h>

// Prototipo para configurar los relojes y los registros MODER/PUPDR de los periféricos
void configurar_pines_gpio(void);
// Prototipo para leer los datos del registro IDR en los puertos especificados
uint8_t leer_estado_pin(volatile GPIO_TypeDef *puerto, uint8_t pin);
// Prototipo para escribir estados (HIGH/LOW) directamente en el registro ODR
void escribir_estado_pin(volatile GPIO_TypeDef *puerto, uint8_t pin, uint8_t estado);

#endif