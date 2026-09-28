#include "SysTick.h"
#define SYSCLK 16000000 // Assuming a system clock of 16 MHz
#define MICROSECONDS_PER_SECOND 1000000

void SysTick_Init(uint32_t us){
    uint32_t period;
    period = (SYSCLK / MICROSECONDS_PER_SECOND) * us;
    SysTick->STCSR = 0; // Disable SysTick
    SysTick->STRVR = period; // Set reload value to maximum (24-bit)
    SysTick->STCVR = 0; // Clear current value
    SysTick->STCSR = 0x05; // Enable SysTick with processor clock
    RCC->APB2ENR |= (0x1UL << 14); // SysTick clock enable ( SYSCFGEN is bit 14 in APB2ENR)
}


void SysTick_Delay_us(uint32_t delay){
    for (uint32_t i=0; i<delay; i++) {
     while(!(SysTick->STCSR & (1<<16))); // Wait until the COUNTFLAG is set, indicating the timer has counted down to zero
    }    
}

void SysTick_Delay_ms(uint32_t delay){
    for (uint32_t i=0; i<delay; i++) {
     SysTick_Delay_us(1000);
    }
}
