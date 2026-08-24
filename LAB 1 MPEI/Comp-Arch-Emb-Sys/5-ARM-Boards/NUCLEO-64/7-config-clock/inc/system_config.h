#ifndef __SYSTEM_CONFIG_H__
#define __SYSTEM_CONFIG_H__
#define STM32F401xE
#include "stm32f4xx.h"
#include "stdint.h"

//System clock
#define SYCLK 84000000 
#define MICROSECONDS_PER_SECOND 1000000

void clock_config(void);

void SysTick_Init(void);
void SysTick_Delay_us(uint32_t delay);
void SysTick_Delay_ms(uint32_t delay);


#endif /* __SYSTEM_CONFIG_H__ */