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

volatile uint32_t period=0, duty=0;
float freq=0.0, dutyper=0.0;

void TIM5_IRQHandler(void){
  if(TIM5->SR&TIM_SR_CC1IF){
    period = TIM5->CCR1;
    if(period!=0) {
      duty = TIM5->CCR2;
    } 
    WRITE_REG_FIELD(TIM5->SR, TIM_SR_CC1IF, 0); // Clear the interrupt flag
  }

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
  WRITE_REG(TIM2->CCR1, 2100-1); //4200-1;
  WRITE_REG_FIELD(TIM2->CCMR1, TIM_CCMR1_OC1M, 6); // Set output compare mode to PWM mode 1
  WRITE_REG_FIELD(TIM2->CCMR1, TIM_CCMR1_OC1PE, 1); // Enable output compare preload for channel 1
  WRITE_REG_FIELD(TIM2->CCER, TIM_CCER_CC1E, 1); // Disable fast mode for channel 1
  WRITE_REG_FIELD(TIM2->CR1, TIM_CR1_DIR, 0); // Counter Up
  WRITE_REG_FIELD(TIM2->CR1, TIM_CR1_ARPE, 1); // Autoreload
  WRITE_REG(TIM2->CNT, 0);                // restart the counter
  WRITE_REG_FIELD(TIM2->CR1, TIM_CR1_CEN, 1); // Enable the timer

}

void TIM5_IC_Init(void){
  
  WRITE_REG_FIELD(RCC->APB1ENR, RCC_APB1ENR_TIM5EN, 1);
  volatile unsigned int dummy;
  dummy =  RCC->APB1ENR;
  dummy =  RCC->APB1ENR;

  /* Timer2 config as base timer*/
  WRITE_REG(TIM5->PSC, 0);             // Prescale to 42MHz
  WRITE_REG(TIM5->ARR, 0xffffffff);         // maximum reload value
 
  WRITE_REG_FIELD(TIM5->CCMR1, TIM_CCMR1_CC1S, 1);  // enable Input capture CH1 on TI1
  WRITE_REG_FIELD(TIM5->CCER, TIM_CCER_CC1P, 0); // Capture configured on rising edge
  WRITE_REG_FIELD(TIM5->CCER, TIM_CCER_CC1NP, 0); // Capture configured on rising edge

  WRITE_REG_FIELD(TIM5->CCMR1, TIM_CCMR1_CC2S, 2); // Enable Input capture CH2 on same TI1
  WRITE_REG_FIELD(TIM5->CCER, TIM_CCER_CC2P, 1); // Enable capture on falling edge
  WRITE_REG_FIELD(TIM5->CCER, TIM_CCER_CC2NP, 0); // Enable capture on falling edge
  
  WRITE_REG_FIELD(TIM5->SMCR, TIM_SMCR_TS, 5); // TI1FP1 selected
  WRITE_REG_FIELD(TIM5->SMCR, TIM_SMCR_SMS, 4); //Reset the Timer on every rising capture event

  WRITE_REG_FIELD(TIM5->CR1, TIM_CR1_DIR, 0); // Counter Up
  WRITE_REG_FIELD(TIM5->CR1, TIM_CR1_ARPE, 1);    // Autoreload, this does not 

  WRITE_REG_FIELD(TIM5->CCER, TIM_CCER_CC1E, 1); // Enable capture CC1
  WRITE_REG_FIELD(TIM5->CCER, TIM_CCER_CC2E, 1); // Enable capture CC2
  WRITE_REG_FIELD(TIM5->DIER, TIM_DIER_CC1IE, 1); // Enable CC1 interrupt (only on rising edge)
  WRITE_REG_FIELD(TIM5->CR1, TIM_CR1_CEN, 1);      // Enable the timer

  NVIC_EnableIRQ(TIM5_IRQn); // Enable the TIM5 IRQ

}

void GPIO_board_config(void){
  GPIO_InitTypeDef GPIO_Init; 
  GPIO_Init.Pin = LED_PIN;
  GPIO_Init.Mode = 2; // Alternate function mode
  GPIO_Init.Pull = 0; // No pull-up or pull-down
  GPIO_Init.Speed = 3; 
  GPIO_Init.Alternate = 1; // Set alternate function to AF1 (TIM2_CH1)
  GPIO_Config(GPIOA,GPIO_Init);

  GPIO_Init.Pin = 0;
  GPIO_Init.Mode = 2; // Alternate function mode
  GPIO_Init.Pull = 0; // No pull-up or pull-down
  GPIO_Init.Alternate = 2; // Set alternate function to AF2 (TIM5_CH1)
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
  
  GPIO_board_config();
  TIM2_PWM_10KHz_Init();
  TIM5_IC_Init();

  while(1)
  {
      freq = 42000000.0 / (float)period; 
      dutyper = ((float) duty / (float) period)*100;
  }


}
