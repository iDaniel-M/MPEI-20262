#ifndef __SYSTICK_H__
#define __SYSTICK_H__

#include "stm32f401.h"


void SysTick_Init(uint32_t us);
void SysTick_Delay_us(uint32_t delay);
void SysTick_Delay_ms(uint32_t delay);



#endif /* __SYSTICK_H__ */