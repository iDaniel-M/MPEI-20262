#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define PERIPHERAL_BASE (0x40000000U) // Peripheral base address for STM32F4 reference manual  pg. 38
#define AHB1_BASE (PERIPHERAL_BASE + 0x20000U) // Peripheral base address for STM32F4 reference manual  pg. 38

#define GPIOA_BASE (AHB1_BASE) // Peripheral base address for STM32F4 reference manual  pg. 38
#define GPIOC_BASE (AHB1_BASE+0x800u) // Peripheral base address for STM32F4 reference manual  pg. 38
#define RCC_BASE   (AHB1_BASE + 0x3800U) // Peripheral base address for STM32F4 reference manual  pg. 38

#define RCC_AHB1ENR ((volatile uint32_t*) (RCC_BASE + 0x30u)) // Peripheral base address for STM32F4 reference manual  pg. 137
#define RCC_AHB1ENR_GPIOAEN (0x00U) // Enable GPIOA clock in RCC_AHB1ENR register (bit 0) for STM32F4 reference manual  pg. 137
#define RCC_AHB1ENR_GPIOCEN (0x02U) // Enable GPIOC clock in RCC_AHB1ENR register (bit 2) for STM32F4 reference manual  pg. 137

#define GPIOA_MODER ((volatile uint32_t*) (GPIOA_BASE)) // Peripheral base address for STM32F4 reference manual  pg. 164
#define GPIO_MODER_MODER5 (10U) // Set GPIOA pin 5 as output in GPIOA_MODER register (bits 10-11) for STM32F4 reference manual  pg. 164

#define GPIOC_MODER ((volatile uint32_t*) (GPIOC_BASE)) // Peripheral base address for STM32F4 reference manual  pg. 164
#define GPIO_MODER_MODER13 (13<<2) // Set GPIOC pin 13 as output in GPIOC_MODER register (bits 26-27) for STM32F4 reference manual  pg. 164


#define GPIOC_IDR ((volatile uint32_t*) (GPIOC_BASE + 0x10u)) // Peripheral base address for STM32F4 reference manual  pg. 164
#define GPIOA_ODR ((volatile uint32_t*) (GPIOA_BASE + 0x14u)) // Peripheral base address for STM32F4 reference manual  pg. 164

#define LED_PIN 5 // Pin 5 corresponds to the on-board LED on the NUCLEO-64 board


int main()
{

  *RCC_AHB1ENR |= (1 << RCC_AHB1ENR_GPIOAEN);
  *RCC_AHB1ENR |= (1 << RCC_AHB1ENR_GPIOCEN);

  // do two dummy reads after enabling the peripheral clock, as per the errata
  volatile uint32_t dummy;
  dummy = *(RCC_AHB1ENR);
  dummy = *(RCC_AHB1ENR);

  *GPIOA_MODER |= (1 << GPIO_MODER_MODER5);
  *GPIOC_MODER &= ~(3 << GPIO_MODER_MODER13);
  
  while(1)
  {

    if((*GPIOC_IDR & (1 << 13)) != 0)
    {
      *GPIOA_ODR |= (1 << LED_PIN);
    }
    else
    {
      *GPIOA_ODR &= ~(1 << LED_PIN);
    }
    
  }


}
