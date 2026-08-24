#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "system_config.h"
#include "stm32f4xx_hal_conf.h"
#include "stm32f4xx_hal.h"


#define LED_PIN 5 // Pin 5 corresponds to the on-board LED on the NUCLEO-64 board
#define BUTTON_PIN 13 // Pin 13 corresponds to the on-board button on the NUCLEO-64 board

/*Declaration of two timers made by software updated every 1ms*/
typedef struct{
  unsigned int sw_tmr1_count;
  unsigned int sw_tmr2_count;
  unsigned int sw_tmr1_period;
  unsigned int sw_tmr2_period;
  unsigned int sw_tmr1_flag;
  unsigned int sw_tmr2_flag;
} SW_Timers;

/* Must be delcared volatile as the timer can update asynchounosly*/
volatile SW_Timers timers;
volatile char Button_status=0;

/*The SysTick_Handler, was already defined as weak during the crt0.s init file, so when we define it here, 
the Vector table is updated whit the new address where the function is allocated, so that when and interrupt happen
the vector table knows where to find the SysTick_Handler*/
void SysTick_Handler(void){
  timers.sw_tmr1_count++;
  timers.sw_tmr2_count++;
  if(timers.sw_tmr1_count==timers.sw_tmr1_period){
    timers.sw_tmr1_count=0;
    timers.sw_tmr1_flag=1;
  }
  if(timers.sw_tmr2_count==timers.sw_tmr2_period){
    timers.sw_tmr2_count=0;
    timers.sw_tmr2_flag=1;
  }
}

void EXTI15_10_IRQHandler(void){
  /*When an interrupt happens, we check first if the pending interrupt asociated to the PC13 is active
  in case that is true, we clear the flag to avoid a new interrupt to happen*/
  if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_13)){
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_13);
    Button_status = ~Button_status;
  }

}


int main()
{
  /*Initialize the timeout period of the SW timers*/
  timers.sw_tmr1_period=500;
  timers.sw_tmr2_period=100;

  HAL_Init(); //Initialices the HAL

  clock_config();

  /*When using the HAL the SysTick can be configure to 1ms as follows, this enable automatically the IRQ*/
  HAL_SYSTICK_Config(((SYCLK/MICROSECONDS_PER_SECOND)*1000));


  /*Enable the CLK to the GPIOA and GPIOC, this needs to be done before the configuration opf the GPIO*/
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Create the GPIO config structure*/
  volatile GPIO_InitTypeDef GPIO_InitStruct;
  /*Config the GPIOA*/
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Config the GPIOC*/
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);


  while(1)
  {
    /*Checks the status of the Button*/
    if (Button_status){
      if(timers.sw_tmr1_flag){
        /*Checks whether the timer reached 500ms, and toggle the LED*/
        timers.sw_tmr1_flag=0;
        HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_5);
      }
    } else {
      if(timers.sw_tmr2_flag){
         /*Checks whether the timer reached 250ms, and toggle the LED*/
        timers.sw_tmr2_flag=0;
        HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_5);
      }
    }
  }


}
