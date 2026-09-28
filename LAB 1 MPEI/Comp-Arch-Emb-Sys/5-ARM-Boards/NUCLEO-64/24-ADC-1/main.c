#include <stdatomic.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "inc/adc.h"
#include "inc/gpio_config.h"
#include "inc/system_config.h"
#include "inc/usart.h"


#define LED_PIN 5 // Pin 5 corresponds to the on-board LED on the NUCLEO-64 board
#define BUTTON_PIN 13 // Pin 13 corresponds to the on-board button on the NUCLEO-64 board
/*Declaration of two timers made by software updated every 1ms*/


/* Must be delcared volatile as the timer can update asynchounosly*/
volatile SW_Timers timers;
volatile uint8_t uart_rx_flag=0;
volatile uint8_t spi_rx_flag=0;
volatile uint8_t uart_rx_data=0;


/*The SysTick_Handler, was already defined as weak during the crt0.s init file, so when we define it here, 
the Vector table is updated whit the new address where the function is allocated, so that when and interrupt happen
the vector table knows where to find the SysTick_Handler*/
void SysTick_Handler(void){
  update_sw_timers(&timers);
}


void USART2_IRQHandler(void){
  if(USART2->SR&USART_SR_RXNE){
    uart_rx_data = uart_getc(USART2);
    uart_rx_flag = 1;
  }
}


int main()
{

  clock_config();
  SysTick_Init(1000);
  SysTick_enable_IrQ(1);
   
  timers.sw_tmr1_period = 500;
  timers.sw_tmr2_period = 1000;
  /*Enable the CLK to the GPIOA and GPIOC, this needs to be done before the configuration opf the GPIO*/

  UART_GPIO_Config(USART2);
  UART_Init(USART2,115200);
  UART_RX_IRQ_EN(USART2,1);
  NVIC_EnableIRQ(USART2_IRQn);

  ADC_GPIO_Config();
  ADC_Init();
  ADC_enable(1);

  uint16_t ADC_VAL[2] = {0,0};
  char u_msg[64]="";

  while(1)
  {
    if(timers.sw_tmr1_flag){
      timers.sw_tmr1_flag=0;
      ADC_Start (1);
      ADC_WaitForConv ();
      ADC_VAL[0] = ADC_GetVal();
      ADC_Start (4);
      ADC_WaitForConv ();
      ADC_VAL[1] = ADC_GetVal();
      sprintf(u_msg,"ADC CH1=%d, CH2=%d\r\n\0",ADC_VAL[0],ADC_VAL[1]);
      uart_puts(USART2,u_msg);
    }
  }

}
