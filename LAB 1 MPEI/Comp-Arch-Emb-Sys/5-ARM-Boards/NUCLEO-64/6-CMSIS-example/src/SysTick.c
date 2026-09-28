#include "SysTick.h"
#define SYSCLK 16000000 // Assuming a system clock of 16 MHz
#define MICROSECONDS_PER_SECOND 1000000

void SysTick_Init(uint32_t us){
    uint32_t period;
    period = (SYSCLK / MICROSECONDS_PER_SECOND) * us - 1;
    WRITE_REG_FIELD(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk, 0); // Disable SysTick
    WRITE_REG_FIELD(SysTick->LOAD, SysTick_LOAD_RELOAD_Msk, period); // Set reload value to maximum (24-bit)
    WRITE_REG_FIELD(SysTick->VAL, SysTick_VAL_CURRENT_Msk, 0); // Clear current value
    WRITE_REG_FIELD(SysTick->CTRL, SysTick_CTRL_CLKSOURCE_Msk, 1); // Set clock source to processor clock
    WRITE_REG_FIELD(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk, 1); // Enable SysTick with processor clock
    WRITE_REG_FIELD(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN_Msk, 1); // SysTick clock enable ( SYSCFGEN is bit 14 in APB2ENR)
}

void SysTick_Delay_us(uint32_t delay){
    for (uint32_t i=0; i<delay; i++) {
     while(!(READ_REG_FIELD(SysTick->CTRL, SysTick_CTRL_COUNTFLAG_Msk))); // Wait until the COUNTFLAG is set, indicating the timer has counted down to zero
    }    
}

void SysTick_Delay_ms(uint32_t delay){
    for (uint32_t i=0; i<delay; i++) {
     SysTick_Delay_us(1000);
    }
}
