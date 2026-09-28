
#include "inc/usart.h"

void UART_GPIO_Config(USART_TypeDef* USART){

    if (USART==USART2){
        WRITE_REG_FIELD(RCC->APB1ENR,RCC_APB1ENR_USART2EN,1);
        WRITE_REG_FIELD(RCC->AHB1ENR,RCC_AHB1ENR_GPIOAEN, 1);
        // do two dummy reads after enabling the peripheral clock, as per the errata
        volatile unsigned int dummy;
        dummy = (RCC->AHB1ENR);
        dummy = (RCC->AHB1ENR);

        /*UART2 TX/RX Pins*/
        WRITE_REG_FIELD(GPIOA->MODER,GPIO_MODER_MODER2,2);
        WRITE_REG_FIELD(GPIOA->AFR[0],GPIO_AFRL_AFRL2,7);
        WRITE_REG_FIELD(GPIOA->MODER,GPIO_MODER_MODER3,2);
        WRITE_REG_FIELD(GPIOA->AFR[0],GPIO_AFRL_AFRL3,7);
    }
    if(USART==USART1){
        //TODO
    }
    //TODO pin selection for other USART

}

void UART_Init(USART_TypeDef* USART, uint32_t baudrate){

  WRITE_REG_FIELD(USART->CR1,USART_CR1_UE,1);
  WRITE_REG_FIELD(USART->CR1,USART_CR1_M,0);
  WRITE_REG_FIELD(USART->CR2,USART_CR2_STOP,0);
  if (USART==USART2){
    USART->BRR = (APB1CLK) / baudrate;
  } else {
    USART->BRR = (APB2CLK) / baudrate;
  }
  WRITE_REG_FIELD(USART->CR1,USART_CR1_TE,1);  
  WRITE_REG_FIELD(USART->CR1,USART_CR1_RE,1);     
}


void UART_RX_IRQ_EN(USART_TypeDef* USART,uint8_t enable){
    WRITE_REG_FIELD(USART->CR1,USART_CR1_RXNEIE,enable); 
}


void uart_putc(USART_TypeDef* USART,char dat){
    while (!(READ_REG_FIELD(USART->SR,USART_SR_TXE)));
    USART->DR = (dat&0xff);
}

char uart_tstc(USART_TypeDef* USART){
    return(READ_REG_FIELD(USART->SR,USART_SR_RXNE));
}

char uart_getc(USART_TypeDef* USART){
    return (char)(USART->DR & 0xffU);
}

void uart_puts(USART_TypeDef* USART,const char* str){
    while (*str)
    {
        uart_putc(USART,*str++);
    }
    
}