#ifndef __ADC_H__
#define __ADC_H__

#include "system_config.h"

void ADC_GPIO_Config(void);
void ADC_Init(void);
void ADC_enable(uint8_t val);
void ADC_Start(uint8_t val);
void ADC_WaitForConv(void);
uint16_t ADC_GetVal(void);

#endif