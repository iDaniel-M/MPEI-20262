#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "inc/system_config.h"
#include "system_config.h"
#include "stm32f4xx_hal_conf.h"
#include "stm32f4xx_hal.h"


#define LED_PIN 5 // Pin 5 corresponds to the on-board LED on the NUCLEO-64 board
#define BUTTON_PIN 13 // Pin 13 corresponds to the on-board button on the NUCLEO-64 board

int main()
{

  HAL_Init();

  clock_config();
  SysTick_Init();

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  volatile GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);



  while(1)
  {
    HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_5);
    if (HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_13)){
      SysTick_Delay_ms(500);
    } else {
      SysTick_Delay_ms(250);
    }
  }


}
