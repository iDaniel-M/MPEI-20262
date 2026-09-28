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

void TIM2_IRQHandler(void){
  WRITE_REG_FIELD(TIM2->SR, TIM_SR_UIF, 0);       // clear the TIM2 flag 
  if (read_pin_state(GPIOC,BUTTON_PIN)){
    toggle_pin_state(GPIOA,LED_PIN);
  }

}

void TIM5_IRQHandler(void){
  WRITE_REG_FIELD(TIM5->SR, TIM_SR_UIF, 0);      // clear the TIM5 flag 
  if (!read_pin_state(GPIOC,BUTTON_PIN)){
    toggle_pin_state(GPIOA,LED_PIN);
  }
}

void Timers_Init(void){
  WRITE_REG_FIELD(RCC->APB1ENR, RCC_APB1ENR_TIM2EN, 1);
  WRITE_REG_FIELD(RCC->APB1ENR, RCC_APB1ENR_TIM5EN, 1);
  volatile unsigned int dummy;
  dummy =  RCC->APB1ENR;
  dummy =  RCC->APB1ENR;

  WRITE_REG_FIELD(TIM2->PSC, TIM_PSC_PSC, 42-1); // Prescaler value to get 1 MHz timer clock (assuming APB2 clock is 42 MHz)
  WRITE_REG(TIM2->ARR, 500000-1); // Set the auto-reload value to achieve a 0.5-second period (500,000 counts at 1 MHz)
  WRITE_REG_FIELD(TIM2->CR1, TIM_CR1_DIR, 0); // Set the timer to count up
  WRITE_REG_FIELD(TIM2->CR1, TIM_CR1_ARPE, 1); // Enable auto-reload preload
  WRITE_REG(TIM2->CNT, 0); // Initialize the counter to 0
  WRITE_REG_FIELD(TIM2->CR1, TIM_CR1_CEN, 1); // Enable the timer
  WRITE_REG_FIELD(TIM2->DIER, TIM_DIER_UIE, 1); // Enable the timer to fire an interrupt

  WRITE_REG_FIELD(TIM5->PSC, TIM_PSC_PSC, 42-1); // Prescaler value to get 1 MHz timer clock (assuming APB2 clock is 42 MHz)
  WRITE_REG(TIM5->ARR, 100000-1); // Set the auto-reload value to achieve a 0.1-second period (100,000 counts at 1 MHz)
  WRITE_REG_FIELD(TIM5->CR1, TIM_CR1_DIR, 0); // Set the timer to count up
  WRITE_REG_FIELD(TIM5->CR1, TIM_CR1_ARPE, 1); // Enable auto-reload preload
  WRITE_REG(TIM5->CNT, 0); // Initialize the counter to 0
  WRITE_REG_FIELD(TIM5->CR1, TIM_CR1_CEN, 1); // Enable the timer
  WRITE_REG_FIELD(TIM5->DIER, TIM_DIER_UIE, 1); // Enable the timer to fire an interrupt

  NVIC_EnableIRQ(TIM2_IRQn);  // Enable the TIM2 IRQ on the NVIC
  NVIC_EnableIRQ(TIM5_IRQn);  // Enable the TIM5 IRQ on the NVIC

}

void GPIO_board_config(void){
  GPIO_InitTypeDef GPIO_Init; 
  GPIO_Init.Pin = LED_PIN;
  GPIO_Init.Mode = 1;
  GPIO_Init.Speed = 3;
  GPIO_Config(GPIOA,GPIO_Init);

  GPIO_Init.Pin = BUTTON_PIN;
  GPIO_Init.Mode = 0;
  GPIO_Init.Pull = 1;
  GPIO_Config(GPIOC,GPIO_Init);
}

int main()
{

  clock_config();
  GPIO_board_config();
  Timers_Init();
  
  while(1)
  {
    /*This example now works using interrupts only, so the main is fully available to run enything else*/
  }


}
