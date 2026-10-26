#ifndef __STM32F446_H__
#define __STM32F446_H__

#include <stdint.h>

/*
 * stm32f446.h - Registros del STM32F446RE escritos a mano (sin CMSIS ni HAL).
 * Solo están los periféricos que usa la serpiente: RCC, GPIO y SysTick.
 * Direcciones: Reference Manual RM0390, tabla "Memory map" (pág. 57).
 *
 * Cada periférico es un "struct" cuyos campos están en el mismo orden que sus
 * registros en memoria (cada registro ocupa 4 bytes). Así GPIOC->ODR es el
 * registro que está en GPIOC_BASE + 0x14.
 */

#define PERIPHERAL_BASE (0x40000000U)                 // inicio de los periféricos
#define AHB1_BASE       (PERIPHERAL_BASE + 0x20000U)  // bus AHB1: GPIO y RCC

/* ===================== GPIO (RM0390 cap. 7) ===================== */
typedef struct {
    volatile uint32_t MODER;   // 0x00: modo de cada pin, 2 bits por pin (00 entrada, 01 salida)
    volatile uint32_t OTYPER;  // 0x04: tipo de salida, 1 bit por pin (0 push-pull)
    volatile uint32_t OSPEEDR; // 0x08: velocidad de la salida, 2 bits por pin
    volatile uint32_t PUPDR;   // 0x0C: resistencias pull-up / pull-down, 2 bits por pin
    volatile uint32_t IDR;     // 0x10: valor leído en cada pin, 1 bit por pin
    volatile uint32_t ODR;     // 0x14: valor que sale por cada pin, 1 bit por pin
    volatile uint32_t BSRR;    // 0x18: poner/borrar bits del ODR (no se usa aquí)
    volatile uint32_t LCKR;    // 0x1C: bloqueo de configuración
    volatile uint32_t AFRL;    // 0x20: función alternativa pines 0..7
    volatile uint32_t AFRH;    // 0x24: función alternativa pines 8..15
} GPIO_TypeDef;

#define GPIOA_BASE (AHB1_BASE + 0x0000U)  // GPIOA en 0x40020000
#define GPIOC_BASE (AHB1_BASE + 0x0800U)  // GPIOC en 0x40020800

#define GPIOA ((volatile GPIO_TypeDef *)GPIOA_BASE)
#define GPIOC ((volatile GPIO_TypeDef *)GPIOC_BASE)

/* ===================== RCC (RM0390 cap. 6) =====================
 * Solo hasta AHB1ENR, que es el único registro que usamos (encender el reloj de los GPIO). */
typedef struct {
    volatile uint32_t CR;        // 0x00
    volatile uint32_t PLLCFGR;   // 0x04
    volatile uint32_t CFGR;      // 0x08
    volatile uint32_t CIR;       // 0x0C
    volatile uint32_t AHB1RSTR;  // 0x10
    volatile uint32_t AHB2RSTR;  // 0x14
    volatile uint32_t AHB3RSTR;  // 0x18
    uint32_t RESERVED0;          // 0x1C (reservado)
    volatile uint32_t APB1RSTR;  // 0x20
    volatile uint32_t APB2RSTR;  // 0x24
    uint32_t RESERVED1[2];       // 0x28, 0x2C (reservados)
    volatile uint32_t AHB1ENR;   // 0x30: encender reloj de periféricos del bus AHB1
} RCC_TypeDef;

#define RCC_BASE (AHB1_BASE + 0x3800U)    // RCC en 0x40023800
#define RCC ((volatile RCC_TypeDef *)RCC_BASE)

#define RCC_AHB1ENR_GPIOAEN (1U << 0)     // bit 0 de AHB1ENR = reloj de GPIOA
#define RCC_AHB1ENR_GPIOCEN (1U << 2)     // bit 2 de AHB1ENR = reloj de GPIOC

/* ===================== SysTick (manual del Cortex-M4) =====================
 * Contador de 24 bits del núcleo que cuenta HACIA ABAJO a 16 MHz (reloj HSI,
 * el que tiene el micro al encender; no lo cambiamos). */
typedef struct {
    volatile uint32_t STCSR;  // 0x00: control y estado
    volatile uint32_t STRVR;  // 0x04: valor de recarga (desde dónde empieza a contar)
    volatile uint32_t STCVR;  // 0x08: valor actual del contador
    volatile uint32_t STCR;   // 0x0C: calibración (no se usa)
} SysTick_TypeDef;

#define SYSTICK_BASE (0xE000E010U)
#define SysTick ((volatile SysTick_TypeDef *)SYSTICK_BASE)

#define SYSTICK_STCSR_ENABLE    (1U << 0)   // bit 0: 1 = el contador cuenta
#define SYSTICK_STCSR_CLKSOURCE (1U << 2)   // bit 2: 1 = usa el reloj del procesador (16 MHz)
#define SYSTICK_STCSR_COUNTFLAG (1U << 16)  // bit 16: se pone en 1 cuando el contador llega a 0

#endif /* __STM32F446_H__ */
