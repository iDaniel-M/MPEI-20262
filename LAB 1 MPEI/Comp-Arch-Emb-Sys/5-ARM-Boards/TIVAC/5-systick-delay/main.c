#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "TM4C123GH6PM.h"
#include "gpio_config.h"
#include "SysTick.h"



char colors[7][3] = {
    {1, 0, 0}, // Red
    {0, 1, 0}, // Green
    {0, 0, 1},  // Blue
    {1, 1, 0}, // Yellow
    {1, 0, 1}, // Magenta
    {0, 1, 1}, // Cyan
    {1, 1, 1}  // White
};


int main()
{

    GPIO_Config(); // Configure GPIO pins for RGB LED and switches
    SysTick_Init();

    uint8_t SW = 0;
    uint8_t i = 0;
  
  while(1)
  {
    SW = read_pin_state(GPIOF, 0) | (read_pin_state(GPIOF, 4) << 4); // Read the state of PF0 and PF4 (switches)
    switch (SW)
    {
    case 0x10: // SW1 pressed
      write_pin_state(GPIOF, RGB_LED_PIN1, 1); // Turn off Green
      write_pin_state(GPIOF, RGB_LED_PIN2, 0); // Turn on Blue
      write_pin_state(GPIOF, RGB_LED_PIN3, 0); // Turn on Red
      break;
    case 0x01: // SW2 pressed
      write_pin_state(GPIOF, RGB_LED_PIN1, 0); // Turn on Blue
      write_pin_state(GPIOF, RGB_LED_PIN2, 1); // Turn on Green
      write_pin_state(GPIOF, RGB_LED_PIN3, 0); // Turn on Red
      break;
    case 0x00: // Both switches pressed
      write_pin_state(GPIOF, RGB_LED_PIN1, 0); // Turn on Blue
      write_pin_state(GPIOF, RGB_LED_PIN2, 0); // Turn on Green
      write_pin_state(GPIOF, RGB_LED_PIN3, 1); // Turn on Red
      break;
    
    default:
          write_pin_state(GPIOF, RGB_LED_PIN1, colors[i][0]); // Set Red
          write_pin_state(GPIOF, RGB_LED_PIN2, colors[i][1]); // Set Green
          write_pin_state(GPIOF, RGB_LED_PIN3, colors[i][2]); // Set Blue
          SysTick_Delay_us(500000);
          i++;
          if (i >= 7) i = 0;
      break;
    }
  }


}
