#include "SysTick.h"
#define SYCLK 16000000 // Assuming a system clock of 16 MHz
#define MICROSECONDS_PER_SECOND 1000000

void SysTick_Init(void){
  SysTick->STCSR = 0; // Disable SysTick
  SysTick->STRVR = 0x00FFFFFF; // Set reload value to maximum (24-bit)
  SysTick->STCVR = 0; // Clear current value
  SysTick->STCSR = 0x05; // Enable SysTick with processor clock

}
void SysTick_Delay_us(uint32_t delay){
    uint32_t ticks = (SYCLK / MICROSECONDS_PER_SECOND) * delay; // Calculate the number of ticks for the specified delay in microseconds
    uint32_t start_value = SysTick->STCVR; // Calculate the reload value for SysTick
    uint32_t elapsed=0;
    uint32_t stop;
    // Wait until the COUNTFLAG is set, indicating the timer has counted down to zero
    while (elapsed < ticks) {
        stop = SysTick->STCVR;
        if (stop > start_value) {
            elapsed = (0x00FFFFFF-stop)+start_value;
        } else {
            elapsed = start_value - stop;
        }
    }

}