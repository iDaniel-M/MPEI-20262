/*
 * regs.h - Definiciones propias de registros del STM32F446RE (sin CMSIS ni HAL).
 *
 * Cada periférico se modela como un struct cuyos campos están en el mismo
 * orden y offset que en el Reference Manual RM0390. "volatile" obliga al
 * compilador a hacer cada lectura/escritura real (no puede optimizarlas),
 * porque el hardware puede cambiar el valor por su cuenta.
 */
#ifndef REGS_H
#define REGS_H

#include <stdint.h>

#define __IO volatile

/* ---------------- RCC (RM0390 sec. 6) @ 0x4002 3800 ---------------- */
typedef struct {
    __IO uint32_t CR;          /* 0x00 */
    __IO uint32_t PLLCFGR;     /* 0x04 */
    __IO uint32_t CFGR;        /* 0x08 */
    __IO uint32_t CIR;         /* 0x0C */
    __IO uint32_t AHB1RSTR;    /* 0x10 */
    __IO uint32_t AHB2RSTR;    /* 0x14 */
    __IO uint32_t AHB3RSTR;    /* 0x18 */
    uint32_t      RES0;        /* 0x1C */
    __IO uint32_t APB1RSTR;    /* 0x20 */
    __IO uint32_t APB2RSTR;    /* 0x24 */
    uint32_t      RES1[2];     /* 0x28-0x2C */
    __IO uint32_t AHB1ENR;     /* 0x30 */
    __IO uint32_t AHB2ENR;     /* 0x34 */
    __IO uint32_t AHB3ENR;     /* 0x38 */
    uint32_t      RES2;        /* 0x3C */
    __IO uint32_t APB1ENR;     /* 0x40 */
    __IO uint32_t APB2ENR;     /* 0x44 */
} RCC_t;
#define RCC ((RCC_t *)0x40023800u)

#define RCC_AHB1ENR_GPIOAEN (1u << 0)
#define RCC_AHB1ENR_GPIOCEN (1u << 2)
#define RCC_APB1ENR_TIM3EN  (1u << 1)
#define RCC_APB1ENR_USART2EN (1u << 17)

/* ---------------- GPIO (RM0390 sec. 7) ---------------- */
typedef struct {
    __IO uint32_t MODER;    /* 0x00  00=entrada 01=salida 10=función alterna 11=analógico */
    __IO uint32_t OTYPER;   /* 0x04  0=push-pull */
    __IO uint32_t OSPEEDR;  /* 0x08  velocidad del flanco */
    __IO uint32_t PUPDR;    /* 0x0C  pull-up / pull-down */
    __IO uint32_t IDR;      /* 0x10  lectura de pines */
    __IO uint32_t ODR;      /* 0x14  latch de salida */
    __IO uint32_t BSRR;     /* 0x18  bits 0-15 ponen en 1, bits 16-31 ponen en 0 (atómico) */
    __IO uint32_t LCKR;     /* 0x1C */
    __IO uint32_t AFR[2];   /* 0x20 AFRL (pines 0-7), 0x24 AFRH (pines 8-15) */
} GPIO_t;
#define GPIOA ((GPIO_t *)0x40020000u)
#define GPIOC ((GPIO_t *)0x40020800u)

/* ---------------- TIM3 (timer de propósito general, RM0390 sec. 18) ---------------- */
typedef struct {
    __IO uint32_t CR1;    /* 0x00 */
    __IO uint32_t CR2;    /* 0x04 */
    __IO uint32_t SMCR;   /* 0x08 */
    __IO uint32_t DIER;   /* 0x0C */
    __IO uint32_t SR;     /* 0x10 */
    __IO uint32_t EGR;    /* 0x14 */
    __IO uint32_t CCMR1;  /* 0x18 */
    __IO uint32_t CCMR2;  /* 0x1C */
    __IO uint32_t CCER;   /* 0x20 */
    __IO uint32_t CNT;    /* 0x24 */
    __IO uint32_t PSC;    /* 0x28 */
    __IO uint32_t ARR;    /* 0x2C */
    uint32_t      RES0;   /* 0x30 (RCR, no existe en TIM3) */
    __IO uint32_t CCR1;   /* 0x34 */
    __IO uint32_t CCR2;   /* 0x38 */
    __IO uint32_t CCR3;   /* 0x3C */
    __IO uint32_t CCR4;   /* 0x40 */
} TIM_t;
#define TIM3 ((TIM_t *)0x40000400u)

#define TIM_CR1_CEN   (1u << 0)
#define TIM_CR1_ARPE  (1u << 7)
#define TIM_DIER_UIE  (1u << 0)
#define TIM_SR_UIF    (1u << 0)
#define TIM_EGR_UG    (1u << 0)
#define TIM_CCER_CC1E (1u << 0)
#define TIM_CCER_CC1P (1u << 1)

/* ---------------- USART2 (RM0390 sec. 25) ---------------- */
typedef struct {
    __IO uint32_t SR;   /* 0x00 */
    __IO uint32_t DR;   /* 0x04 */
    __IO uint32_t BRR;  /* 0x08 */
    __IO uint32_t CR1;  /* 0x0C */
    __IO uint32_t CR2;  /* 0x10 */
    __IO uint32_t CR3;  /* 0x14 */
} USART_t;
#define USART2 ((USART_t *)0x40004400u)

#define USART_SR_TXE  (1u << 7)
#define USART_CR1_TE  (1u << 3)
#define USART_CR1_UE  (1u << 13)

/* ---------------- Núcleo Cortex-M4 ---------------- */
#define NVIC_ISER0 (*(__IO uint32_t *)0xE000E100u)  /* habilita IRQ 0..31 */
#define SCB_CPACR  (*(__IO uint32_t *)0xE000ED88u)  /* acceso al coprocesador (FPU) */

/* DWT: contador de ciclos de CPU, usado para retardos exactos (pruebas.c).
   ARM v7-M Architecture Reference Manual, C1.8 (DWT) y C1.6.5 (DEMCR). */
#define DEMCR      (*(__IO uint32_t *)0xE000EDFCu)  /* bit 24 TRCENA: habilita DWT/ITM */
#define DWT_CTRL   (*(__IO uint32_t *)0xE0001000u)  /* bit 0 CYCCNTENA: arranca el contador */
#define DWT_CYCCNT (*(__IO uint32_t *)0xE0001004u)  /* ciclos de CPU transcurridos */

#define TIM3_IRQn 29u   /* posición de TIM3 en la tabla de vectores (RM0390 tabla 38) */

#endif
