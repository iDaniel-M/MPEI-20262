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

#define LED_PIN 5 // Pin 5 corresponds to the on-board LED on the NUCLEO-64 board
#define BUTTON_PIN 13 // Pin 13 corresponds to the on-board button on the NUCLEO-64 board
#define USARTx USART2
/*Declaration of two timers made by software updated every 1ms*/


/* Must be delcared volatile as the timer can update asynchounosly*/
volatile SW_Timers timers;
volatile uint8_t uart_rx_flag=0;
volatile uint8_t spi_rx_flag=0;
uint8_t spi_master[1]="";
uint8_t spi_slave[1] ="";
uint8_t spi_slave_rcv[1]="";
uint8_t i=1;


/*The SysTick_Handler, was already defined as weak during the crt0.s init file, so when we define it here, 
the Vector table is updated whit the new address where the function is allocated, so that when and interrupt happen
the vector table knows where to find the SysTick_Handler*/
void SysTick_Handler(void){
  update_sw_timers(&timers);
}


void SPI3_IRQHandler(void){
  if(SPI3->SR&SPI_SR_RXNE){
    spi_slave_rcv[0] = SPI_Slave_Receive(SPI3);
    SPI_Slave_Transmit(SPI3,spi_slave_rcv[0]);
    spi_rx_flag = 1;
  }
}

void USART2_IRQHandler(void){
  if(USART2->SR&USART_SR_RXNE){
    spi_master[0] = uart_getc(USART2);
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

  UART_GPIO_Config(USART2);
  SPI_GPIO_Config(SPI2);
  SPI_GPIO_Config(SPI3);
}


int main()
{

  clock_config();
  SysTick_Init(1000);
  SysTick_enable_IrQ(1);
   
  timers.sw_tmr1_period = 2000;
  timers.sw_tmr2_period = 1000;
  /*Enable the CLK to the GPIOA and GPIOC, this needs to be done before the configuration opf the GPIO*/
  

  GPIO_board_config();

  UART_Init(USART2,115200);
  UART_RX_IRQ_EN(USART2,1);
  NVIC_EnableIRQ(USART2_IRQn);

  SPI_Master_Init(SPI2);
  SPI_Enable(SPI2,1);

  SPI_Slave_Init(SPI3);
  SPI_Enable(SPI3,1);
  SPI_Slave_IRQ(SPI3,1);
  NVIC_EnableIRQ(SPI3_IRQn);

  write_pin_state(GPIOB,9,1);
  
  while(1)
  {
    if(uart_rx_flag){
      uart_rx_flag = 0;
      uart_puts(USART2,"1-UART-RX/SPI-M-TX: ");
      uart_putc(USART2,spi_master[0]);
      uart_puts(USART2,"\r\n");
      write_pin_state(GPIOB,9,0);
      SPI_Master_Tranfer(SPI2,spi_master,1);
      write_pin_state(GPIOB,9,1);
    }
    if(spi_rx_flag){
      spi_rx_flag = 0;
      uart_puts(USART2,"2-SPI-S-RX/UART-TX: ");
      uart_putc(USART2,spi_slave_rcv[0]);
      uart_puts(USART2,"\r\n");
    }
    if(timers.sw_tmr1_flag){
      timers.sw_tmr1_flag=0;
      /*Here I can Add a timestamp*/
      uart_puts(USART2,"UART-SPI loopback example\r\n\r\n");
    }
  }


}
