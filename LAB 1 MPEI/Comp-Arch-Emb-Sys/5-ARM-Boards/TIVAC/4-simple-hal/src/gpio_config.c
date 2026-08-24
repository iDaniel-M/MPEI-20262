#include "gpio_config.h"




void GPIO_Config(void)
{
  
    // 1. Enable clock for Port F
    SYSCTL->RCGCGPIO |= 0x20; // Enable clock for Port F
    
    // 2. Wait for the clock to be ready
    while((SYSCTL->PRGPIO & 0x20) == 0) {}; // Wait for the clock to be ready
    
    // 3. Set PF1, PF2, and PF3 as digital output pins
    GPIOF->GPIODIR = 0; // Set all pins of Port F as input initially
    GPIOF->GPIODIR |= (1 << RGB_LED_PIN1) | (1 << RGB_LED_PIN2) | (1 << RGB_LED_PIN3); // Set PF1, PF2, and PF3 as output pins
    
    
    // 4. Enable digital function for PF1, PF2, PF3

    GPIOF->GPIODEN |= (1 << RGB_LED_PIN1) | (1 << RGB_LED_PIN2) | (1 << RGB_LED_PIN3); // Enable digital function for PF1, PF2, and PF3
    GPIOF->GPIOLOCK = 0x4C4F434B; // Unlock GPIO Port F
    GPIOF->GPIOCR |= 0x11; // Allow changes to PF0 and PF4 (switches)

    GPIOF->GPIODEN |= 0x11; // Enable digital function for PF0 and PF4 (switches)
    GPIOF->GPIOPUR |= 0x11; // Enable pull-up resistors for PF0 and PF4 (switches)


}

uint8_t read_pin_state(volatile GPIO_TypeDef *GPIOx, uint8_t pin){
  return (GPIOx->GPIODATA[255] & (1 << pin)) ? 1 : 0;
}

void write_pin_state(volatile GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t state){
  if(state){
    GPIOx->GPIODATA[255] |= (1 << pin);
  } else {
    GPIOx->GPIODATA[255] &= ~(1 << pin);
  }
}