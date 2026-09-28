#ifndef _MAX7219_H_
#define _MAX7219_H_

#include "system_config.h"

void MAX7219_GPIOConfig(void);
void max7219_config(void);
void max7219_print(uint8_t * Matrix);

#endif