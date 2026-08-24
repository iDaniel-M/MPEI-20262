#ifndef __SYSTICK_H__
#define __SYSTICK_H__

#include "stm32f4xx.h"


void SysTick_Init(void);
void SysTick_Delay_us(uint32_t delay);


#endif /* __SYSTICK_H__ */