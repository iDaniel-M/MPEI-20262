#ifndef __SYSTICK_H__
#define __SYSTICK_H__

#include "stm32f4xx.h"

#define WRITE_REG_FIELD(REG, MASK, VAL) \
    (REG = (REG & ~MASK) | ((VAL << POSITION_VAL(MASK)) & MASK))

#define READ_REG_FIELD(REG, MASK) \
    ((REG & MASK) >> POSITION_VAL(MASK))

void SysTick_Init(uint32_t us);
void SysTick_Delay_us(uint32_t delay);
void SysTick_Delay_ms(uint32_t delay);


#endif /* __SYSTICK_H__ */