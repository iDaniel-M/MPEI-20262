#ifndef STM32F401_H
#define STM32F401_H

#include <stdint.h> // Importante mantenerlo para las traducciones estructurales de registros

// Referencias de hardware extraídas del Reference Manual del STM32 (Mapa de memoria fijo)
#define PERIPH_BASE         (0x40000000U) // Inicio de memoria de periféricos
#define AHB1PERIPH_BASE     (PERIPH_BASE + 0x00020000U) // Bus de altas prestaciones donde viven los GPIO

// Direcciones absolutas de los tres puertos físicos implicados y del control de reloj (RCC)
#define GPIOA_BASE          (AHB1PERIPH_BASE + 0x0000U)
#define GPIOB_BASE          (AHB1PERIPH_BASE + 0x0400U)
#define GPIOC_BASE          (AHB1PERIPH_BASE + 0x0800U)
#define RCC_BASE            (AHB1PERIPH_BASE + 0x3800U)

// Plantilla que imita bit a bit los saltos de memoria de los registros GPIO oficiales
// Usamos "volatile" de forma imperativa para prohibir que el compilador ignore escrituras que considere "redundantes"
typedef struct {
    volatile uint32_t MODER;   // [Offset 0x00] Modo general de operación
    volatile uint32_t OTYPER;  // [Offset 0x04] Modo eléctrico (Push-Pull o Drain Abierto)
    volatile uint32_t OSPEEDR; // [Offset 0x08] Velocidad de transistores en la salida
    volatile uint32_t PUPDR;   // [Offset 0x0C] Resistencias débiles de anclaje (Pull Up/Down)
    volatile uint32_t IDR;     // [Offset 0x10] Registro esclavo de datos entrantes en los pines físicos
    volatile uint32_t ODR;     // [Offset 0x14] Registro esclavo de datos proyectados hacia los pines físicos
    volatile uint32_t BSRR;    // [Offset 0x18] Registro Bit Set/Reset para atomicidad en la manipulación
    volatile uint32_t LCKR;    // [Offset 0x1C] Interbloqueo que congela configuraciones (No usado acá)
    volatile uint32_t AFRL;    // [Offset 0x20] Ruteo alternativo de pines lógicos para los conectores del 0 al 7
    volatile uint32_t AFRH;    // [Offset 0x24] Ruteo alternativo de pines lógicos para los conectores del 8 al 15
} GPIO_TypeDef;

// Plantilla de las entrañas del árbol de reloj general del STM32
typedef struct {
    volatile uint32_t CR;
    volatile uint32_t PLLCFGR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t AHB1RSTR;
    volatile uint32_t AHB2RSTR;
    uint32_t RESERVED0[2]; // Pads vacíos documentados en el datasheet
    volatile uint32_t APB1RSTR;
    volatile uint32_t APB2RSTR;
    uint32_t RESERVED1[2]; // Pads vacíos
    volatile uint32_t AHB1ENR; // El registro más importante: Control de poder al grupo AHB1 (Puertos)
    volatile uint32_t AHB2ENR;
    uint32_t RESERVED2[2];
    volatile uint32_t APB1ENR;
    volatile uint32_t APB2ENR;
} RCC_TypeDef;

// Conversiones finales explícitas en formato de punteros para poder usarlas como -> en el código C
#define RCC                 ((RCC_TypeDef *) RCC_BASE)
#define GPIOA               ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB               ((GPIO_TypeDef *) GPIOB_BASE)
#define GPIOC               ((GPIO_TypeDef *) GPIOC_BASE)

#endif