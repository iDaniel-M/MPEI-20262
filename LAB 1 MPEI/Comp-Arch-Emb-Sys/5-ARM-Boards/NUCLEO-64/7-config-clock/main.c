#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f401xe.h"
#include "inc/gpio_config.h"
#include "inc/system_config.h"


#define LED_PIN 5 // Pin 5 corresponds to the on-board LED on the NUCLEO-64 board
#define BUTTON_PIN 13 // Pin 13 corresponds to the on-board button on the NUCLEO-64 board


void GPIO_board_config(void)
{
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

  SysTick_Init(1);

  while(1)
  {

    if(read_pin_state(GPIOC, BUTTON_PIN) != 0)
    {
      write_pin_state(GPIOA, LED_PIN, 1);
      SysTick_Delay_ms(500);
      write_pin_state(GPIOA, LED_PIN, 0);
      SysTick_Delay_ms(500);
    }
    else
    {
      write_pin_state(GPIOA, LED_PIN, 0);
      SysTick_Delay_ms(250);
      write_pin_state(GPIOA, LED_PIN, 1);
      SysTick_Delay_ms(250);
    }
    
  }


}
