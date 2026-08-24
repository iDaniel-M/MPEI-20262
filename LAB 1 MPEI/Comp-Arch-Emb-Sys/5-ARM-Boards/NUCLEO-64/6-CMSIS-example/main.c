#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f401xe.h"
#include "gpio_config.h"
#include "SysTick.h"


#define LED_PIN 5 // Pin 5 corresponds to the on-board LED on the NUCLEO-64 board
#define BUTTON_PIN 13 // Pin 13 corresponds to the on-board button on the NUCLEO-64 board

int main()
{

  GPIO_Config();
  SysTick_Init();

  while(1)
  {

    if(read_pin_state(GPIOC, BUTTON_PIN) != 0)
    {
      write_pin_state(GPIOA, LED_PIN, 1);
      SysTick_Delay_us(500000);
      write_pin_state(GPIOA, LED_PIN, 0);
      SysTick_Delay_us(500000);
    }
    else
    {
      write_pin_state(GPIOA, LED_PIN, 0);
      SysTick_Delay_us(250000);
      write_pin_state(GPIOA, LED_PIN, 1);
      SysTick_Delay_us(250000);
    }
    
  }


}
