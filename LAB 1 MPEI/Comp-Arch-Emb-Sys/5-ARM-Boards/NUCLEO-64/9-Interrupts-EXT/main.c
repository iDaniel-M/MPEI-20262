#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "inc/system_config.h"
#include "inc/gpio_config.h"


#define LED_PIN 5 // Pin 5 corresponds to the on-board LED on the NUCLEO-64 board
#define BUTTON_PIN 13 // Pin 13 corresponds to the on-board button on the NUCLEO-64 board

/* Must be delcared volatile as the timer can update asynchounosly*/
volatile SW_Timers timers;
volatile char Button_status=0;

/*The SysTick_Handler, was already defined as weak during the crt0.s init file, so when we define it here, 
the Vector table is updated whit the new address where the function is allocated, so that when and interrupt happen
the vector table knows where to find the SysTick_Handler*/
void SysTick_Handler(void){
  update_sw_timers(&timers);
}

void EXTI15_10_IRQHandler(void){
  /*When an interrupt happens, we check first if the pending interrupt asociated to the PC13 is active
  in case that is true, we clear the flag to avoid a new interrupt to happen*/
  if (EXTI->PR & EXTI_PR_PR13){
    WRITE_REG_FIELD(EXTI->PR, EXTI_PR_PR13,1); // Clear the pending interrupt flag for EXTI13
    Button_status = ~Button_status;
  }

}


void EXTI_Init(void){
  GPIO_InitTypeDef GPIO_Init; 
  GPIO_Init.Pin = LED_PIN;
  GPIO_Init.Mode = 1;
  GPIO_Init.Speed = 3;
  GPIO_Config(GPIOA,GPIO_Init);

  GPIO_Init.Pin = BUTTON_PIN;
  GPIO_Init.Mode = 0;
  GPIO_Init.Pull = 1;
  GPIO_Config(GPIOC,GPIO_Init);

  WRITE_REG_FIELD(SYSCFG->EXTICR[3], SYSCFG_EXTICR4_EXTI13_Msk, 2); // Set EXTI13 to be triggered by PC13 (EXTICR4 is for pins 12-15, and we want pin 13, which is in the second half of the register)
  WRITE_REG_FIELD(EXTI->RTSR, EXTI_RTSR_TR13_Msk, 1); // Enable rising edge trigger for EXTI13
  WRITE_REG_FIELD(EXTI->IMR, EXTI_IMR_MR13,1); // Unmask EXTI13 interrupt

  NVIC_SetPriority(EXTI15_10_IRQn, 1);
  NVIC_EnableIRQ(EXTI15_10_IRQn);
}


int main()
{
  /*Initialize the timeout period of the SW timers*/
  timers.sw_tmr1_period=500;
  timers.sw_tmr2_period=100;

  clock_config();

  /*This is a custom SysTick IRQ config*/
  SysTick_Init(1000);
  SysTick_enable_IrQ(1);

  EXTI_Init();
  
  while(1)
  {
    /*Checks the status of the Button*/
    if (Button_status){
      if(timers.sw_tmr1_flag){
        /*Checks whether the timer reached 500ms, and toggle the LED*/
        timers.sw_tmr1_flag=0;
        toggle_pin_state(GPIOA,LED_PIN);
      }
    } else {
      if(timers.sw_tmr2_flag){
         /*Checks whether the timer reached 250ms, and toggle the LED*/
        timers.sw_tmr2_flag=0;
        toggle_pin_state(GPIOA,LED_PIN);
      }
    }
  }


}
