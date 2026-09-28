#include "gpio_config.h"


void GPIO_clock_enable(volatile GPIO_TypeDef *GPIOx){
  if(GPIOx==GPIOA){
    WRITE_REG_FIELD(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN_Msk, 1);
  } else if(GPIOx==GPIOB){
    WRITE_REG_FIELD(RCC->AHB1ENR, RCC_AHB1ENR_GPIOBEN_Msk, 1);
  } else if(GPIOx==GPIOC){
    WRITE_REG_FIELD(RCC->AHB1ENR, RCC_AHB1ENR_GPIOCEN_Msk, 1);
  } else if(GPIOx==GPIOD){
    WRITE_REG_FIELD(RCC->AHB1ENR, RCC_AHB1ENR_GPIODEN_Msk, 1);
  } else if(GPIOx==GPIOE){
    WRITE_REG_FIELD(RCC->AHB1ENR, RCC_AHB1ENR_GPIOEEN_Msk, 1);
  } else if(GPIOx==GPIOH){
    WRITE_REG_FIELD(RCC->AHB1ENR, RCC_AHB1ENR_GPIOHEN_Msk, 1);
  }
  volatile unsigned int dummy;
  dummy = (RCC->AHB1ENR);
  dummy = (RCC->AHB1ENR);
}

void GPIO_Config(volatile GPIO_TypeDef *GPIOx, GPIO_InitTypeDef Config)
{
  GPIO_clock_enable(GPIOx);
  WRITE_REG_FIELD(GPIOx->MODER, GPIO_MODER_MODER0_Msk << (Config.Pin * 2), Config.Mode);
  WRITE_REG_FIELD(GPIOx->OSPEEDR, GPIO_OSPEEDR_OSPEED0_Msk << (Config.Pin * 2), Config.Speed);
  WRITE_REG_FIELD(GPIOx->PUPDR, GPIO_PUPDR_PUPD0_Msk << (Config.Pin * 2), Config.Pull);
  if (Config.Pin < 8){
      WRITE_REG_FIELD(GPIOx->AFR[0], GPIO_AFRL_AFSEL0_Msk << ((Config.Pin % 8) * 4), Config.Alternate);
  } else {
      WRITE_REG_FIELD(GPIOx->AFR[1], GPIO_AFRH_AFSEL8_Msk << ((Config.Pin % 8) * 4), Config.Alternate);
  }
}

uint32_t read_pin_state(volatile GPIO_TypeDef *GPIOx, uint32_t pin){
  return (GPIOx->IDR & (1<<pin)) ? 1 : 0;
}

void write_pin_state(volatile GPIO_TypeDef *GPIOx, uint32_t pin, uint32_t state){
  if(state){
    GPIOx->ODR |= (1<<pin);
  } else {
    GPIOx->ODR &= ~(1<<pin);
  }
}

void toggle_pin_state(volatile GPIO_TypeDef *GPIOx, uint32_t pin){
    GPIOx->ODR ^= (1<<pin);
}