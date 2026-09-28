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
volatile uint8_t spi_slave_received;
volatile uint8_t spi_received_data;
uint8_t spi_slave[12] ="Slave-MSG\r\n\0";
uint8_t spi_slave_rcv[12]="";
uint8_t i=1;


/*The SysTick_Handler, was already defined as weak during the crt0.s init file, so when we define it here, 
the Vector table is updated whit the new address where the function is allocated, so that when and interrupt happen
the vector table knows where to find the SysTick_Handler*/
void SysTick_Handler(void){
  update_sw_timers(&timers);
}


void SPI3_IRQHandler(void){
  if(SPI3->SR&SPI_SR_RXNE){
    spi_slave_rcv[i] = SPI_Slave_Receive(SPI3);
    i++;
    SPI_Slave_Transmit(SPI3,spi_slave[i]);
  }
}


void GPIOConfig(void){

  WRITE_REG_FIELD(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN, 1);
  WRITE_REG_FIELD(RCC->AHB1ENR, RCC_AHB1ENR_GPIOBEN, 1);
  WRITE_REG_FIELD(RCC->AHB1ENR, RCC_AHB1ENR_GPIOCEN, 1);
  // do two dummy reads after enabling the peripheral clock, as per the errata
  volatile unsigned int dummy;
  dummy = (RCC->AHB1ENR);
  dummy = (RCC->AHB1ENR);
  
  /*UART2 TX/RX Pins*/
  WRITE_REG_FIELD(GPIOA->MODER,GPIO_MODER_MODER2,2);
  WRITE_REG_FIELD(GPIOA->AFR[0],GPIO_AFRL_AFRL2,7);
  WRITE_REG_FIELD(GPIOA->MODER,GPIO_MODER_MODER3,2);
  WRITE_REG_FIELD(GPIOA->AFR[0],GPIO_AFRL_AFRL3,7);

  /*SPI2 Pins*/
  WRITE_REG_FIELD(GPIOB->MODER,GPIO_MODER_MODER9,1); //SPI1_NSS
  WRITE_REG_FIELD(GPIOB->OSPEEDR,GPIO_OSPEEDER_OSPEEDR9,3);
  WRITE_REG_FIELD(GPIOB->MODER,GPIO_MODER_MODER15,2); //SPI1_MOSI
  WRITE_REG_FIELD(GPIOB->AFR[1],GPIO_AFRH_AFRH7,5);
  WRITE_REG_FIELD(GPIOB->OSPEEDR,GPIO_OSPEEDER_OSPEEDR15,3);
  WRITE_REG_FIELD(GPIOB->MODER,GPIO_MODER_MODER14,2); //SPI1_MISO
  WRITE_REG_FIELD(GPIOB->AFR[1],GPIO_AFRH_AFRH6,5);
  WRITE_REG_FIELD(GPIOB->OSPEEDR,GPIO_OSPEEDER_OSPEEDR14,3);
  WRITE_REG_FIELD(GPIOB->MODER,GPIO_MODER_MODER13,2); //SPI1_SCk
  WRITE_REG_FIELD(GPIOB->AFR[1],GPIO_AFRH_AFRH5,5);
  WRITE_REG_FIELD(GPIOB->OSPEEDR,GPIO_OSPEEDER_OSPEEDR13,3);
  

  /*SPI3 Pins*/
  WRITE_REG_FIELD(GPIOA->MODER,GPIO_MODER_MODER4,2); //SPI3_NSS
  WRITE_REG_FIELD(GPIOA->AFR[0],GPIO_AFRL_AFRL4,6);
  WRITE_REG_FIELD(GPIOA->OSPEEDR,GPIO_OSPEEDER_OSPEEDR4,3);
  WRITE_REG_FIELD(GPIOC->MODER,GPIO_MODER_MODER12,2); //SPI3_MOSI
  WRITE_REG_FIELD(GPIOC->AFR[1],GPIO_AFRH_AFRH4,6);
  WRITE_REG_FIELD(GPIOC->OSPEEDR,GPIO_OSPEEDER_OSPEEDR12,3);
  WRITE_REG_FIELD(GPIOC->MODER,GPIO_MODER_MODER11,2); //SPI3_MISO
  WRITE_REG_FIELD(GPIOC->AFR[1],GPIO_AFRH_AFRH3,6);
  WRITE_REG_FIELD(GPIOC->OSPEEDR,GPIO_OSPEEDER_OSPEEDR11,3);
  WRITE_REG_FIELD(GPIOC->MODER,GPIO_MODER_MODER10,2); //SPI3_SCk
  WRITE_REG_FIELD(GPIOC->AFR[1],GPIO_AFRH_AFRH2,6);
  WRITE_REG_FIELD(GPIOC->OSPEEDR,GPIO_OSPEEDER_OSPEEDR10,3);

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

  UART_GPIO_Config(USART2);
  SPI_GPIO_Config(SPI2);
  SPI_GPIO_Config(SPI3);
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
  UART_Init(USART2,115200);
  
  SPI_Master_Init(SPI2);
  SPI_Slave_Init(SPI3);
  SPI_Enable(SPI2,1);
  SPI_Enable(SPI3,1);
  SPI_Slave_IRQ(SPI3,1);

  NVIC_EnableIRQ(SPI3_IRQn);

  write_pin_state(GPIOB,9,1);
  //SPI_Enable(SPI4,0);
  uint8_t spi_master[12]="MasterMSG\r\n\0";
  
  while(1)
  {
    if(timers.sw_tmr1_flag){
      timers.sw_tmr1_flag=0;
      i=0;
      sprintf(spi_master,"MasterMSG\r\n\0");
      write_pin_state(GPIOB,9,0);
      SPI_Slave_Transmit(SPI3,spi_slave[i]);
      SPI_Master_Tranfer(SPI2,spi_master,12);
      write_pin_state(GPIOB,9,1);

      uart_puts(USART2,"The SPI Master Sent: ");
      uart_puts(USART2,spi_slave_rcv);

      uart_puts(USART2,"The SPI Master Received: ");
      uart_puts(USART2,spi_master);

      uart_puts(USART2,"The SPI Slave Sent: ");
      uart_puts(USART2,spi_slave);

      uart_puts(USART2,"The SPI Slave Received: ");
      uart_puts(USART2,spi_slave_rcv);
    }
  }


}
