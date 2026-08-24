#include "system_config.h"


void SysTick_Init(uint32_t us){
    uint32_t period;
    period = (SYCLK / MICROSECONDS_PER_SECOND) * us;
    SysTick->CTRL = 0; // Disable SysTick
    SysTick->LOAD = period; // Set reload value to maximum (24-bit)
    SysTick->VAL = 0; // Clear current value
    SysTick->CTRL = 0x05; // Enable SysTick with processor clock
    RCC->APB2ENR |= (RCC_APB2ENR_SYSCFGEN);
}

void SysTick_enable_IrQ(char val){
    if(val){
        SysTick->CTRL |= 2;
    }else{
        SysTick->CTRL &= ~(2);
    }
}



void SysTick_Delay_us(uint32_t delay){
    uint32_t ticks = (SYCLK / MICROSECONDS_PER_SECOND) * delay; // Calculate the number of ticks for the specified delay in microseconds
    uint32_t start_value ; // Calculate the reload value for SysTick
    uint32_t elapsed=0;
    uint32_t stop;
    start_value = SysTick->VAL; 
    // Wait until the COUNTFLAG is set, indicating the timer has counted down to zero
    while (elapsed < ticks) {
        stop = SysTick->VAL;
        if (stop > start_value) {
            elapsed = (SysTick->LOAD-stop)+start_value;
        } else {
            elapsed = start_value - stop;
        }
    }

}

void SysTick_Delay_ms(uint32_t delay){
    for (int i=0; i<delay; i++) {
     SysTick_Delay_us(1000);
    }
}



void clock_config(void){

    #if SYCLK == 84000000

    FLASH->ACR |= FLASH_ACR_LATENCY_3WS; // Flash Latency must be adjusted considering voltage and CPU clockspeed

    RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLSRC); 
    RCC->PLLCFGR |= (RCC_PLLCFGR_PLLM_Msk & 16); //M=16 
    RCC->PLLCFGR |= (RCC_PLLCFGR_PLLN_Msk & 336); //N=336
    RCC->PLLCFGR |= (RCC_PLLCFGR_PLLP_Msk & 1); //P=4
    RCC->PLLCFGR |= (RCC_PLLCFGR_PLLQ_Msk & 7); //Q=7

    RCC->CR |= RCC_CR_PLLON_Msk;  // enable the PLL
    while (! (RCC->CR & RCC_CR_PLLRDY_Msk)); // Wait for the PLL be ready

    RCC->CFGR |= (RCC_CFGR_SW_PLL << RCC_CFGR_SW_Pos); //Select PLL clock as main system clock
    while (! (RCC->CFGR & RCC_CFGR_SWS_PLL)); // Wait for the system to switch the clk;
    #else
    #endif
    SystemCoreClockUpdate();

}