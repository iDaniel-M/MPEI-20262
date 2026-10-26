#ifndef __STM32F446_REGS_H__
#define __STM32F446_REGS_H__
/*
 * stm32f446_regs.h - Registros del STM32F446RE escritos a mano.
 *
 * El ejercicio del profesor usa "stm32f4xx.h" de CMSIS, pero en el curso CMSIS está
 * prohibido. Este archivo lo reemplaza: define SOLO lo que usan sus librerías, con
 * LOS MISMOS NOMBRES, para que gpio_config.c, system_config.c y usart.c queden igual.
 *
 * Direcciones: RM0390 (Reference Manual del F446), tabla "Memory map".
 * Cada periférico es un struct con sus registros en orden (4 bytes cada uno).
 * Una "máscara" (_Msk) tiene en 1 los bits de un campo del registro.
 */
#include <stdint.h>

/* ======================= GPIO (RM0390 cap. 7) ======================= */
typedef struct {
    volatile uint32_t MODER;    // 0x00 modo: 2 bits/pin (00 entrada, 01 salida, 10 función alterna)
    volatile uint32_t OTYPER;   // 0x04 tipo de salida: 1 bit/pin (0 push-pull)
    volatile uint32_t OSPEEDR;  // 0x08 velocidad: 2 bits/pin
    volatile uint32_t PUPDR;    // 0x0C pull: 2 bits/pin (00 nada, 01 pull-up, 10 pull-down)
    volatile uint32_t IDR;      // 0x10 lectura de los pines
    volatile uint32_t ODR;      // 0x14 salida de los pines
    volatile uint32_t BSRR;     // 0x18 bits 0-15 ponen el pin en 1, bits 16-31 lo ponen en 0
    volatile uint32_t LCKR;     // 0x1C bloqueo
    volatile uint32_t AFR[2];   // 0x20 AFR[0] = pines 0-7, 0x24 AFR[1] = pines 8-15 (4 bits/pin)
} GPIO_TypeDef;

#define GPIOA ((volatile GPIO_TypeDef *)0x40020000U)
#define GPIOB ((volatile GPIO_TypeDef *)0x40020400U)
#define GPIOC ((volatile GPIO_TypeDef *)0x40020800U)
#define GPIOD ((volatile GPIO_TypeDef *)0x40020C00U)
#define GPIOE ((volatile GPIO_TypeDef *)0x40021000U)
#define GPIOH ((volatile GPIO_TypeDef *)0x40021C00U)

#define GPIO_MODER_MODER0_Msk   (0x3U << 0)   // 2 bits del pin 0 (se desplaza 2*pin)
#define GPIO_MODER_MODER2       (0x3U << 4)   // pin 2 (PA2 = TX)
#define GPIO_MODER_MODER3       (0x3U << 6)   // pin 3 (PA3 = RX)
#define GPIO_OSPEEDR_OSPEED0_Msk (0x3U << 0)
#define GPIO_PUPDR_PUPD0_Msk    (0x3U << 0)
#define GPIO_AFRL_AFSEL0_Msk    (0xFU << 0)   // 4 bits del pin 0 en AFR[0]
#define GPIO_AFRH_AFSEL8_Msk    (0xFU << 0)   // 4 bits del pin 8 en AFR[1]
#define GPIO_AFRL_AFRL2         (0xFU << 8)   // pin 2 en AFR[0]
#define GPIO_AFRL_AFRL3         (0xFU << 12)  // pin 3 en AFR[0]

/* ======================= RCC (RM0390 cap. 6) ======================= */
typedef struct {
    volatile uint32_t CR;        // 0x00 encender osciladores y PLL
    volatile uint32_t PLLCFGR;   // 0x04 configuración del PLL
    volatile uint32_t CFGR;      // 0x08 elegir reloj del sistema y divisores de buses
    volatile uint32_t CIR;       // 0x0C
    volatile uint32_t AHB1RSTR;  // 0x10
    volatile uint32_t AHB2RSTR;  // 0x14
    volatile uint32_t AHB3RSTR;  // 0x18
    uint32_t RESERVED0;          // 0x1C
    volatile uint32_t APB1RSTR;  // 0x20
    volatile uint32_t APB2RSTR;  // 0x24
    uint32_t RESERVED1[2];       // 0x28, 0x2C
    volatile uint32_t AHB1ENR;   // 0x30 reloj de GPIOx
    volatile uint32_t AHB2ENR;   // 0x34
    volatile uint32_t AHB3ENR;   // 0x38
    uint32_t RESERVED2;          // 0x3C
    volatile uint32_t APB1ENR;   // 0x40 reloj de TIM3, USART2
    volatile uint32_t APB2ENR;   // 0x44 reloj de SYSCFG
} RCC_TypeDef;

#define RCC ((volatile RCC_TypeDef *)0x40023800U)

#define RCC_AHB1ENR_GPIOAEN_Msk (1U << 0)
#define RCC_AHB1ENR_GPIOBEN_Msk (1U << 1)
#define RCC_AHB1ENR_GPIOCEN_Msk (1U << 2)
#define RCC_AHB1ENR_GPIODEN_Msk (1U << 3)
#define RCC_AHB1ENR_GPIOEEN_Msk (1U << 4)
#define RCC_AHB1ENR_GPIOHEN_Msk (1U << 7)
#define RCC_AHB1ENR_GPIOAEN     RCC_AHB1ENR_GPIOAEN_Msk
#define RCC_APB1ENR_TIM3EN      (1U << 1)
#define RCC_APB1ENR_USART2EN    (1U << 17)
#define RCC_APB2ENR_SYSCFGEN_Msk (1U << 14)

#define RCC_CR_PLLON      (1U << 24)       // 1 = encender el PLL
#define RCC_CR_PLLRDY     (1U << 25)       // el hardware lo pone en 1 cuando el PLL está estable
#define RCC_PLLCFGR_PLLM  (0x3FU << 0)     // divisor de entrada M (bits 0-5)
#define RCC_PLLCFGR_PLLN  (0x1FFU << 6)    // multiplicador N (bits 6-14)
#define RCC_PLLCFGR_PLLP  (0x3U << 16)     // divisor de salida P (00=/2, 01=/4, ...)
#define RCC_PLLCFGR_PLLQ  (0xFU << 24)     // divisor Q (USB), no se usa
#define RCC_CFGR_SW       (0x3U << 0)      // reloj del sistema: 00 HSI, 10 PLL
#define RCC_CFGR_SW_PLL   (0x2U)           // valor 10 = PLL
#define RCC_CFGR_SWS_PLL  (0x2U << 2)      // bit 3: el sistema ya está usando el PLL
#define RCC_CFGR_HPRE     (0xFU << 4)      // divisor del bus AHB (0 = /1)
#define RCC_CFGR_PPRE1    (0x7U << 10)     // divisor del bus APB1 (4 = /2)
#define RCC_CFGR_PPRE2    (0x7U << 13)     // divisor del bus APB2 (0 = /1)

/* ======================= FLASH (RM0390 cap. 3) ======================= */
typedef struct {
    volatile uint32_t ACR;       // 0x00 latencia: ciclos de espera al leer la FLASH
} FLASH_TypeDef;

#define FLASH ((volatile FLASH_TypeDef *)0x40023C00U)
#define FLASH_ACR_LATENCY     (0xFU << 0)
#define FLASH_ACR_LATENCY_3WS (3U)          // 3 ciclos de espera (de sobra para 84 MHz)

/* ======================= TIM3 (RM0390 cap. 17) ======================= */
typedef struct {
    volatile uint32_t CR1;    // 0x00 control: CEN (bit 0) arranca, ARPE (bit 7) preload de ARR
    volatile uint32_t CR2;    // 0x04
    volatile uint32_t SMCR;   // 0x08
    volatile uint32_t DIER;   // 0x0C UIE (bit 0) = interrupción al desbordar
    volatile uint32_t SR;     // 0x10 UIF (bit 0) = bandera de desborde
    volatile uint32_t EGR;    // 0x14 UG (bit 0) = cargar PSC/ARR ya
    volatile uint32_t CCMR1;  // 0x18 modo de los canales 1 y 2
    volatile uint32_t CCMR2;  // 0x1C
    volatile uint32_t CCER;   // 0x20 CC1E (bit 0) = sacar el canal 1 por el pin
    volatile uint32_t CNT;    // 0x24 contador
    volatile uint32_t PSC;    // 0x28 prescaler: el contador avanza a reloj / (PSC+1)
    volatile uint32_t ARR;    // 0x2C cuenta hasta ARR y vuelve a 0
    volatile uint32_t RCR;    // 0x30
    volatile uint32_t CCR1;   // 0x34 comparación del canal 1 (PWM)
} TIM_TypeDef;

#define TIM3 ((volatile TIM_TypeDef *)0x40000400U)

/* ======================= USART (RM0390 cap. 25) ======================= */
typedef struct {
    volatile uint32_t SR;     // 0x00 estado: TXE (bit 7) listo para enviar, RXNE (bit 5) llegó un dato
    volatile uint32_t DR;     // 0x04 dato enviado / recibido
    volatile uint32_t BRR;    // 0x08 velocidad: BRR = reloj del bus / baudios
    volatile uint32_t CR1;    // 0x0C UE (bit 13), M (bit 12), RXNEIE (bit 5), TE (bit 3), RE (bit 2)
    volatile uint32_t CR2;    // 0x10 STOP (bits 12-13): bits de parada
    volatile uint32_t CR3;    // 0x14
    volatile uint32_t GTPR;   // 0x18
} USART_TypeDef;

#define USART1 ((USART_TypeDef *)0x40011000U)
#define USART2 ((USART_TypeDef *)0x40004400U)

#define USART_SR_RXNE   (1U << 5)
#define USART_SR_TXE    (1U << 7)
#define USART_CR1_RE    (1U << 2)
#define USART_CR1_TE    (1U << 3)
#define USART_CR1_RXNEIE (1U << 5)
#define USART_CR1_M     (1U << 12)
#define USART_CR1_UE    (1U << 13)
#define USART_CR2_STOP  (0x3U << 12)

/* ======================= SysTick (manual del Cortex-M4) ======================= */
typedef struct {
    volatile uint32_t CTRL;   // 0x00 ENABLE (bit 0), TICKINT (bit 1), CLKSOURCE (bit 2), COUNTFLAG (bit 16)
    volatile uint32_t LOAD;   // 0x04 valor de recarga (24 bits)
    volatile uint32_t VAL;    // 0x08 valor actual
    volatile uint32_t CALIB;  // 0x0C
} SysTick_Type;

#define SysTick ((volatile SysTick_Type *)0xE000E010U)

#define SysTick_CTRL_ENABLE_Msk    (1U << 0)
#define SysTick_CTRL_TICKINT_Msk   (1U << 1)
#define SysTick_CTRL_CLKSOURCE_Msk (1U << 2)
#define SysTick_CTRL_COUNTFLAG_Msk (1U << 16)
#define SysTick_LOAD_RELOAD_Msk    (0xFFFFFFU)
#define SysTick_VAL_CURRENT_Msk    (0xFFFFFFU)

/* ======================= NVIC (manual del Cortex-M4) =======================
 * ISER[n]: escribir 1 en el bit k habilita la interrupción 32*n + k. */
#define NVIC_ISER ((volatile uint32_t *)0xE000E100U)

#define TIM3_IRQn   29   // posición de TIM3 en la tabla de vectores (RM0390 tabla 38)
#define USART2_IRQn 38   // posición de USART2

static inline void NVIC_EnableIRQ(uint32_t irq)
{
    NVIC_ISER[irq / 32] = (1U << (irq % 32));
}

#endif /* __STM32F446_REGS_H__ */
