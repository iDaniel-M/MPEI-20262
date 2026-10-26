#ifndef _USART_H_
#define _USART_H_

#include "system_config.h"

void UART_GPIO_Config(USART_TypeDef* USART);

void UART_Init(USART_TypeDef* USART, uint32_t baudrate);
void UART_RX_IRQ_EN(USART_TypeDef* USART,uint8_t enable);


void uart_putc(USART_TypeDef* USART,char dat);
char uart_tstc(USART_TypeDef* USART);
char uart_getc(USART_TypeDef* USART);

void uart_puts(USART_TypeDef* USART,const char* str);



#endif 