#include "system_config.h"

void update_sw_timers(volatile SW_Timers* timer){
  timer->sw_tmr1_count++;
  timer->sw_tmr2_count++;
  if(timer->sw_tmr1_count==timer->sw_tmr1_period){
    timer->sw_tmr1_count=0;
    timer->sw_tmr1_flag=1;
  }
  if(timer->sw_tmr2_count==timer->sw_tmr2_period){
    timer->sw_tmr2_count=0;
    timer->sw_tmr2_flag=1;
  }
}

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

void clock_config(void){

    #if SYSCLK == 84000000

    WRITE_REG_FIELD(FLASH->ACR,FLASH_ACR_LATENCY,FLASH_ACR_LATENCY_3WS); // Flash Latency must be adjusted considering voltage and CPU clockspeed

    WRITE_REG_FIELD(RCC->PLLCFGR,RCC_PLLCFGR_PLLM,16);  // M=16
    WRITE_REG_FIELD(RCC->PLLCFGR,RCC_PLLCFGR_PLLN,336); // N=336
    WRITE_REG_FIELD(RCC->PLLCFGR,RCC_PLLCFGR_PLLP,1); // P=4
    WRITE_REG_FIELD(RCC->PLLCFGR,RCC_PLLCFGR_PLLQ,7); // Q=7


    WRITE_REG_FIELD(RCC->CFGR,RCC_CFGR_PPRE1,4); // APB1 clocked to 42 MHz
    WRITE_REG_FIELD(RCC->CFGR,RCC_CFGR_PPRE2,0);
    WRITE_REG_FIELD(RCC->CFGR,RCC_CFGR_HPRE,0);

    WRITE_REG_FIELD(RCC->CR,RCC_CR_PLLON,1); // enable the PLL
    while (! (READ_REG_FIELD(RCC->CR,RCC_CR_PLLRDY))); // Wait for the PLL be ready

    WRITE_REG_FIELD(RCC->CFGR,RCC_CFGR_SW,RCC_CFGR_SW_PLL);
    //RCC->CFGR |= (RCC_CFGR_SW_PLL << RCC_CFGR_SW_Pos); //Select PLL clock as main system clock
    while (!(READ_REG_FIELD(RCC->CFGR,RCC_CFGR_SWS_PLL))); // Wait for the system to switch the clk;
    
    #else
    #endif
   

}