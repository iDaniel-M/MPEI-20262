#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "inc/gpio_config.h"
#include "inc/system_config.h"
#include "inc/gpio_config.h"

#define LED_PIN 5 // Pin 5 corresponds to the on-board LED on the NUCLEO-64 board
#define BUTTON_PIN 13 // Pin 13 corresponds to the on-board button on the NUCLEO-64 board


/* Must be delcared volatile as the timer can update asynchounosly*/
volatile SW_Timers timers;

/*The SysTick_Handler, was already defined as weak during the crt0.s init file, so when we define it here, 
the Vector table is updated whit the new address where the function is allocated, so that when and interrupt happen
the vector table knows where to find the SysTick_Handler*/
void SysTick_Handler(void){
  update_sw_timers(&timers);
}

void TIM2_PWM_10KHz_Init(void){
  /*Enable the clock to the timers 2 and 5*/
  
  WRITE_REG_FIELD(RCC->APB1ENR, RCC_APB1ENR_TIM2EN, 1);
  volatile unsigned int dummy;
  dummy =  RCC->APB1ENR;
  dummy =  RCC->APB1ENR;

  /* Timer2 config as base timer*/
  WRITE_REG(TIM2->PSC, 0);             // Prescale to 1MHz
  WRITE_REG(TIM2->ARR, 4200-1);         // Timeout at 500ms
  WRITE_REG(TIM2->CCR1, 10); //4200-1;
  WRITE_REG_FIELD(TIM2->CCMR1, TIM_CCMR1_OC1M, 6); // Set output compare mode to PWM mode 1
  WRITE_REG_FIELD(TIM2->CCMR1, TIM_CCMR1_OC1PE, 1); // Enable output compare preload for channel 1
  WRITE_REG_FIELD(TIM2->CCER, TIM_CCER_CC1E, 1); // Disable fast mode for channel 1
  WRITE_REG_FIELD(TIM2->CR1, TIM_CR1_DIR, 0); // Counter Up
  WRITE_REG_FIELD(TIM2->CR1, TIM_CR1_ARPE, 1); // Autoreload
  WRITE_REG(TIM2->CNT, 0);                // restart the counter
  WRITE_REG_FIELD(TIM2->CR1, TIM_CR1_CEN, 1); // Enable the timer

}

void GPIO_board_config(void){
  GPIO_InitTypeDef GPIO_Init; 
  GPIO_Init.Pin = LED_PIN;
  GPIO_Init.Mode = 2; // Alternate function mode
  GPIO_Init.Pull = 0; // No pull-up or pull-down
  GPIO_Init.Speed = 3; 
  GPIO_Init.Alternate = 1; // Set alternate function to AF1 (TIM2_CH1)
  GPIO_Config(GPIOA,GPIO_Init);

  GPIO_Init.Pin = BUTTON_PIN;
  GPIO_Init.Mode = 0;
  GPIO_Init.Pull = 1;
  GPIO_Config(GPIOC,GPIO_Init);
}

int main()
{

  clock_config();
  SysTick_Init(1000);
  SysTick_enable_IrQ(1);
   
  timers.sw_tmr1_period = 10;
  /*Enable the CLK to the GPIOA and GPIOC, this needs to be done before the configuration opf the GPIO*/
  
  GPIO_board_config();

  TIM2_PWM_10KHz_Init();
  uint32_t duty_cycle=50;
  while(1)
  {

    if(timers.sw_tmr1_flag){
      timers.sw_tmr1_flag = 0;
      duty_cycle+=50;
      if(duty_cycle>=4200){
        duty_cycle=50;
      }
      WRITE_REG(TIM2->CCR1, duty_cycle);
    }  

  }


}
