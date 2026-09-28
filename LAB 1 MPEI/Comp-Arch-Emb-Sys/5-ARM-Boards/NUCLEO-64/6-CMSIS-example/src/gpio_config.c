#include "gpio_config.h"




void GPIO_Config(void)
{
  WRITE_REG_FIELD(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN_Msk, 1); // Enable GPIOA clock in RCC_AHB1ENR register (bit 0)
  WRITE_REG_FIELD(RCC->AHB1ENR, RCC_AHB1ENR_GPIOCEN_Msk, 1); // Enable GPIOC clock in RCC_AHB1ENR register (bit 2)

  // do two dummy reads after enabling the peripheral clock, as per the errata
  volatile unsigned int dummy;
  dummy = (RCC->AHB1ENR);
  dummy = (RCC->AHB1ENR);


  WRITE_REG_FIELD(GPIOA->MODER, GPIO_MODER_MODER5_Msk, 1); // Set PA5 as output
  WRITE_REG_FIELD(GPIOC->MODER, GPIO_MODER_MODER13_Msk, 0); // Set PC13 as input

}

uint8_t read_pin_state(volatile GPIO_TypeDef *GPIOx, uint8_t pin){
  return (GPIOx->IDR & (1 << pin)) ? 1 : 0;
}

void write_pin_state(volatile GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t state){
  if(state){
    GPIOx->ODR |= (1 << pin);
  } else {
    GPIOx->ODR &= ~(1 << pin);
  }
}