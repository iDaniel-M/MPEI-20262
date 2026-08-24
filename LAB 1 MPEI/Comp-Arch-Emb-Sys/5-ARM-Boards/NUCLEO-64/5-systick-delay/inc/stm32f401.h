#ifndef __STM32F401_H__
#define __STM32F401_H__
#include <stdint.h>

#define PERIPHERAL_BASE (0x40000000U) // Peripheral base address for STM32F4 reference manual  pg. 38
#define AHB1_BASE (PERIPHERAL_BASE + 0x20000U) // Peripheral base address for STM32F4 reference manual  pg. 38

typedef struct {
    volatile unsigned int MODER0 : 2;
    volatile unsigned int MODER1 : 2;
    volatile unsigned int MODER2 : 2;
    volatile unsigned int MODER3 : 2;
    volatile unsigned int MODER4 : 2;
    volatile unsigned int MODER5 : 2;
    volatile unsigned int MODER6 : 2;
    volatile unsigned int MODER7 : 2;
    volatile unsigned int MODER8 : 2;
    volatile unsigned int MODER9 : 2;
    volatile unsigned int MODER10 : 2;
    volatile unsigned int MODER11 : 2;
    volatile unsigned int MODER12 : 2;
    volatile unsigned int MODER13 : 2;
    volatile unsigned int MODER14 : 2;
    volatile unsigned int MODER15 : 2;
} MODER_Bits;

typedef struct {
    union {
        volatile unsigned int MODER;   // GPIO port mode register
        volatile MODER_Bits MODER_Bits; // GPIO port mode register bitfield
    };
    //volatile unsigned int MODER;   // GPIO port mode register
    volatile unsigned int OTYPER;  // GPIO port output type register
    volatile unsigned int OSPEEDR; // GPIO port output speed register
    volatile unsigned int PUPDR;   // GPIO port pull-up/pull-down register
    volatile unsigned int IDR;     // GPIO port input data register
    volatile unsigned int ODR;     // GPIO port output data register
    volatile unsigned int BSRR;    // GPIO port bit set/reset register
    volatile unsigned int LCKR;    // GPIO port configuration lock register
    volatile unsigned int AFRL;  // GPIO alternate function registers (AFRL)
    volatile unsigned int AFRH;  // GPIO alternate function registers (AFRH)
} GPIO_TypeDef;


typedef struct {
    volatile unsigned int CR;       // RCC clock control register
    volatile unsigned int PLLCFGR;  // RCC PLL configuration register
    volatile unsigned int CFGR;     // RCC clock configuration register
    volatile unsigned int CIR;      // RCC clock interrupt register
    volatile unsigned int AHB1RSTR; // RCC AHB1 peripheral reset register
    volatile unsigned int AHB2RSTR; // RCC AHB2 peripheral reset register
    unsigned int RESERVED0[2];         // Reserved
    volatile unsigned int APB1RSTR; // RCC APB1 peripheral reset register
    volatile unsigned int APB2RSTR; // RCC APB2 peripheral reset register
    unsigned int RESERVED1[2];      // Reserved
    volatile unsigned int AHB1ENR;  // RCC AHB1 peripheral clock enable register
    volatile unsigned int AHB2ENR;  // RCC AHB2 peripheral clock enable register
    unsigned int RESERVED2[2];         // Reserved
    volatile unsigned int APB1ENR;  // RCC APB1 peripheral clock enable register
    volatile unsigned int APB2ENR;  // RCC APB2 peripheral clock enable register
    unsigned int RESERVED3[2];         // Reserved
    volatile unsigned int AHB1LPENR; // RCC AHB1 peripheral clock enable in low power mode register
    volatile unsigned int AHB2LPENR; // RCC AHB2 peripheral clock enable in low power mode register
    unsigned int RESERVED4[2];         // Reserved
    volatile unsigned int APB1LPENR; // RCC APB1 peripheral clock enable in low power mode register
    volatile unsigned int APB2LPENR; // RCC APB2 peripheral clock enable in low power mode register
    unsigned int RESERVED5[2];         // Reserved
    volatile unsigned int BDCR;     // RCC Backup domain control register
    volatile unsigned int CSR;      // RCC clock control & status register
    unsigned int RESERVED6[2];         // Reserved
    volatile unsigned int SSCGR;    // RCC spread spectrum clock generation register
    volatile unsigned int PLLI2SCFGR; // RCC PLLI2S configuration register
    unsigned int RESERVED7[1];         // Reserved
    volatile unsigned int DCKCFGR;  // RCC Dedicated Clocks Configuration Register
} RCC_TypeDef;


#define GPIOA_BASE (AHB1_BASE) // Peripheral base address for STM32F4 reference manual  pg. 38
#define GPIOC_BASE (AHB1_BASE + 0x800u) // Peripheral base address for STM32F4 reference manual  pg. 38
#define GPIOB_BASE (AHB1_BASE + 0x400u) // Peripheral base address for STM32F4 reference manual  pg. 38
#define GPIOD_BASE (AHB1_BASE + 0xC00u) // Peripheral base address for STM32F4 reference manual  pg. 38
#define GPIOE_BASE (AHB1_BASE + 0x1000u) // Peripheral base address for STM32F4 reference manual  pg. 38
#define GPIOH_BASE (AHB1_BASE + 0x1C00u) // Peripheral base address for STM32F4 reference manual  pg. 38

#define RCC_BASE   (AHB1_BASE + 0x3800U) // Peripheral base address for STM32F4 reference manual  pg. 38


#define GPIOA ((volatile GPIO_TypeDef *)GPIOA_BASE)
#define GPIOB ((volatile GPIO_TypeDef *)GPIOB_BASE)
#define GPIOC ((volatile GPIO_TypeDef *)GPIOC_BASE)
#define GPIOD ((volatile GPIO_TypeDef *)GPIOD_BASE)
#define GPIOE ((volatile GPIO_TypeDef *)GPIOE_BASE)
#define GPIOH ((volatile GPIO_TypeDef *)GPIOH_BASE)

#define RCC ((volatile RCC_TypeDef *)RCC_BASE)

#define SYS_BASE (0xE000E000U) 
#define SYSTICK_BASE (SYS_BASE + 0x10U) // Peripheral base address for STM32F4 reference manual  pg. 38

typedef struct {
    volatile unsigned int STCSR; // SysTick Control and Status Register
    volatile unsigned int STRVR; // SysTick Reload Value Register
    volatile unsigned int STCVR; // SysTick Current Value Register
    volatile unsigned int STCR; // SysTick Calibration Value Register
} SysTick_TypeDef;

#define SysTick ((volatile SysTick_TypeDef *)SYSTICK_BASE)

#endif /* __STM32F401_H__ */