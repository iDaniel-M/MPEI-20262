#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define PERIPHERALS_BASE 0x40000000

#define SYSCTL_RCGCGPIO_R   (((volatile uint32_t *) (PERIPHERALS_BASE + 0x000FE608))) // GPIO Run Mode Clock Gating Control
#define SYSCTL_PRGPIO_R     (((volatile uint32_t *) (PERIPHERALS_BASE + 0x000FEA08))) // GPIO Peripheral Ready
#define GPIO_PORTF_DIR_R    (((volatile uint32_t *) (PERIPHERALS_BASE + 0x00025400))) // GPIO Direction
#define GPIO_PORTF_DEN_R    (((volatile uint32_t *) (PERIPHERALS_BASE + 0x0002551C))) // GPIO Digital Enable
#define GPIO_PORTF_LOCK_R   (((volatile uint32_t *) (PERIPHERALS_BASE + 0x00025520))) // GPIO Lock
#define GPIO_PORTF_CR_R     (((volatile uint32_t *) (PERIPHERALS_BASE + 0x00025524))) // GPIO Commit
#define GPIO_PORTF_PUR_R    (((volatile uint32_t *) (PERIPHERALS_BASE + 0x00025510))) // GPIO Pull-Up Select
#define GPIO_PORTF_MASK_R   0xFF // Mask for GPIO Port F
#define GPIO_PORTF_DATA_R   (((volatile uint32_t *) (PERIPHERALS_BASE + 0x00025000 + (GPIO_PORTF_MASK_R << 2)))) // GPIO Data Register


#define RGB_LED_PIN1 1 // PF1
#define RGB_LED_PIN2 2 // PF2
#define RGB_LED_PIN3 3 // PF3

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

  // 1. Enable clock for Port F
    *SYSCTL_RCGCGPIO_R |= 0x20; 
    
    // 2. Wait for the clock to be ready
    while((*SYSCTL_PRGPIO_R & 0x20) == 0) {};
    
    // 3. Set PF1, PF2, and PF3 as digital output pins
    *GPIO_PORTF_DIR_R = 0; //
    *GPIO_PORTF_DIR_R |= (1 << RGB_LED_PIN1) | (1 << RGB_LED_PIN2) | (1 << RGB_LED_PIN3);
    
    
    // 4. Enable digital function for PF1, PF2, PF3
    *GPIO_PORTF_DEN_R |= (1 << RGB_LED_PIN1) | (1 << RGB_LED_PIN2) | (1 << RGB_LED_PIN3);
    *GPIO_PORTF_LOCK_R = 0x4C4F434B; // Unlock GPIO Port F
    *GPIO_PORTF_CR_R |= 0x11; // Allow changes to PF0

    *GPIO_PORTF_DEN_R |= 0x11; // Enable digital function for PF0 and PF4 (switches)
    *GPIO_PORTF_PUR_R |= 0x11; // Enable pull-up resistors for PF0 and PF4 (switches)
    uint8_t SW = 0;
    uint8_t i = 0;
  
  while(1)
  {
    SW = *GPIO_PORTF_DATA_R & 0x11; // Read the state of PF0 and PF4 (switches)
    switch (SW)
    {
    case 0x10: // SW1 pressed
      *GPIO_PORTF_DATA_R = (1 << RGB_LED_PIN1); // Red
      break;
    case 0x01: // SW2 pressed
      *GPIO_PORTF_DATA_R = (1 << RGB_LED_PIN2); // Green
      break;
    case 0x00: // Both switches pressed
      *GPIO_PORTF_DATA_R = (1 << RGB_LED_PIN3); // Blue
      break;
    
    default:
          *GPIO_PORTF_DATA_R = (colors[i][0] << RGB_LED_PIN1) | (colors[i][1] << RGB_LED_PIN2) | (colors[i][2] << RGB_LED_PIN3);
          for (uint32_t j = 0; j < 500000; j++);
          i++;
          if (i >= 7) i = 0;
      break;
    }
  }


}
