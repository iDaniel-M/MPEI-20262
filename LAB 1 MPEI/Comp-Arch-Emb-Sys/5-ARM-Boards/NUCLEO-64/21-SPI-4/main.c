#include <stdatomic.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "inc/gpio_config.h"
#include "inc/spi.h"
#include "inc/system_config.h"
#include "inc/gpio_config.h"
#include "inc/usart.h"
#include "max7219.h"

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


void GPIO_board_config(void){
  GPIO_InitTypeDef GPIO_Init; 
  GPIO_Init.Pin = LED_PIN;
  GPIO_Init.Mode = 1; // Output function mode
  GPIO_Init.Pull = 0; // No pull-up or pull-down
  GPIO_Init.Speed = 3; 
  GPIO_Config(GPIOA,GPIO_Init);

  GPIO_Init.Pin = BUTTON_PIN;
  GPIO_Init.Mode = 0;
  GPIO_Init.Pull = 1;
  GPIO_Config(GPIOC,GPIO_Init);

}

uint8_t data[8] ={
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00
};

typedef struct 
{
  uint8_t x;
  uint8_t y;
} pointd;

pointd point;


void draw_point(uint8_t * Matrix, pointd point){
  for(uint8_t i=0; i<8; i++){
    Matrix[i] = 0;
    if(point.y == i){
      Matrix[i] |= (1<<point.x);
    }
  }
  max7219_print(Matrix);
}

int main()
{

  clock_config();
  SysTick_Init(1000);
  SysTick_enable_IrQ(1);
   
  timers.sw_tmr1_period = 1000;
  timers.sw_tmr2_period = 1000;
  /*Enable the CLK to the GPIOA and GPIOC, this needs to be done before the configuration opf the GPIO*/
  

  GPIO_board_config();

  UART_GPIO_Config(USART2);
  UART_Init(USART2,115200);
  UART_RX_IRQ_EN(USART2,1);
  NVIC_EnableIRQ(USART2_IRQn);
  
  max7219_config();

  max7219_print(data);

  uint8_t idx=0;
  uint8_t p_message[20];
  point.x = 3,
  point.y = 3;

  draw_point(data,point);
 uint8_t direction = 'w';
  while(1)
  {
    if(timers.sw_tmr1_flag){
      timers.sw_tmr1_flag = 0;
      toggle_pin_state(GPIOA,LED_PIN);
      if(direction=='w'){
        if(point.y<7){
          point.y++;
        }
      } else if (direction=='s') {
        if(point.y>0){
          point.y--;
        }
      } else if (direction=='a') {
        if(point.x>0){
          point.x--;
        }
      }else if (direction=='d') {
        if(point.x<7){
          point.x++;
        }
      }
      sprintf(p_message,"x=%d,y=%d\n\r\0",point.x,point.y);
      uart_puts(USART2,p_message);
      draw_point(data,point);
    }
    
    if(uart_rx_flag){
      uart_rx_flag = 0;
      direction = uart_rx_data;
      sprintf(p_message,"x=%d,y=%d\n\r\0",point.x,point.y);
      uart_puts(USART2,p_message);
      draw_point(data,point);
    }
  }


}
