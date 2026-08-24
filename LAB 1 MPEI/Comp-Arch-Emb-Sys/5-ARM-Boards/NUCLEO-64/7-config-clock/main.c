#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f401xe.h"
#include "gpio_config.h"
#include "system_config.h"


#define LED_PIN 5 // Pin 5 corresponds to the on-board LED on the NUCLEO-64 board
#define BUTTON_PIN 13 // Pin 13 corresponds to the on-board button on the NUCLEO-64 board

int main()
{

  clock_config();
  GPIO_Config();
  SysTick_Init();

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
