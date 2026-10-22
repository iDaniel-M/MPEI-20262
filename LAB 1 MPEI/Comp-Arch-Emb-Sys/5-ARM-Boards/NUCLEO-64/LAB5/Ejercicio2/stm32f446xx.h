#ifndef STM32F446XX_H
#define STM32F446XX_H

#include <stdint.h>

/* Mapeo de Registros del Puerto de Control de Reloj (RCC) */
typedef struct {
    volatile uint32_t CR;
    volatile uint32_t PLLCFGR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t AHB1RSTR;
    volatile uint32_t AHB2RSTR;
    volatile uint32_t AHB3RSTR;
    uint32_t RESERVED0;
    volatile uint32_t APB1RSTR;
    volatile uint32_t APB2RSTR;
    uint32_t RESERVED1[2];
    volatile uint32_t AHB1ENR;
    volatile uint32_t AHB2ENR;
    volatile uint32_t AHB3ENR;
    uint32_t RESERVED2;
    volatile uint32_t APB1ENR;
    volatile uint32_t APB2ENR;
} RCC_TypeDef;

/* Mapeo de Registros GPIO */
typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFR[2];
} GPIO_TypeDef;

/* Mapeo de Registros de Timers de Propósito General (TIM2 / TIM3) */
typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SMCR;
    volatile uint32_t DIER;
    volatile uint32_t SR;
    volatile uint32_t EGR;
    volatile uint32_t CCMR1;
    volatile uint32_t CCMR2;
    volatile uint32_t CCER;
    volatile uint32_t CNT;
    volatile uint32_t PSC;
    volatile uint32_t ARR;
    uint32_t RESERVED0;
    volatile uint32_t CCR1;
    volatile uint32_t CCR2;
    volatile uint32_t CCR3;
    volatile uint32_t CCR4;
} TIM_TypeDef;

/* Mapeo de Registros SysTick del Núcleo ARM Cortex-M4 */
typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t LOAD;
    volatile uint32_t VAL;
    volatile uint32_t CALIB;
} SysTick_TypeDef;

/* Base de Direcciones de Memoria de Periféricos (STM32F446RE) */
#define PERIPH_BASE           (0x40000000UL)
#define APB1PERIPH_BASE       PERIPH_BASE
#define AHB1PERIPH_BASE       (PERIPH_BASE + 0x00020000UL)

#define RCC_BASE              (AHB1PERIPH_BASE + 0x3800UL)
#define GPIOA_BASE            (AHB1PERIPH_BASE + 0x0000UL)
#define GPIOB_BASE            (AHB1PERIPH_BASE + 0x0400UL)
#define GPIOC_BASE            (AHB1PERIPH_BASE + 0x0800UL)

#define TIM2_BASE             (APB1PERIPH_BASE + 0x0000UL)
#define TIM3_BASE             (APB1PERIPH_BASE + 0x0400UL)

#define SCS_BASE              (0xE000E000UL)
#define SysTick_BASE          (SCS_BASE + 0x0010UL)
#define NVIC_BASE             (SCS_BASE + 0x0100UL)

#define RCC                   ((RCC_TypeDef *) RCC_BASE)
#define GPIOA                 ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB                 ((GPIO_TypeDef *) GPIOB_BASE)
#define GPIOC                 ((GPIO_TypeDef *) GPIOC_BASE)
#define TIM2                  ((TIM_TypeDef *) TIM2_BASE)
#define TIM3                  ((TIM_TypeDef *) TIM3_BASE)
#define SysTick               ((SysTick_TypeDef *) SysTick_BASE)

/* Máscaras de Bits RCC */
#define RCC_AHB1ENR_GPIOAEN   (1UL << 0)
#define RCC_AHB1ENR_GPIOBEN   (1UL << 1)
#define RCC_AHB1ENR_GPIOCEN   (1UL << 2)
#define RCC_APB1ENR_TIM2EN    (1UL << 0)
#define RCC_APB1ENR_TIM3EN    (1UL << 1)

/* Configuración de Pines GPIO */
#define GPIO_MODER_MODE0_1    (1UL << 1)

/* Timers y Flags */
#define TIM_CCMR1_CC1S_0      (1UL << 0)
#define TIM_CCER_CC1E         (1UL << 0)
#define TIM_DIER_CC1IE        (1UL << 1)
#define TIM_DIER_UIE          (1UL << 0)
#define TIM_CR1_CEN           (1UL << 0)
#define TIM_SR_CC1IF          (1UL << 1)
#define TIM_SR_UIF            (1UL << 0)

/* SysTick Flags */
#define SysTick_CTRL_CLKSOURCE_Msk (1UL << 2)
#define SysTick_CTRL_ENABLE_Msk    (1UL << 0)

/* Control de Interrupciones NVIC (ARM Cortex-M4) */
#define TIM2_IRQn 28
#define TIM3_IRQn 29

static inline void NVIC_EnableIRQ(int32_t IRQn) {
    volatile uint32_t *nvic_iser = (volatile uint32_t *)(NVIC_BASE + 0x00);
    nvic_iser[IRQn >> 5] = (1UL << (IRQn & 0x1F));
}

#endif