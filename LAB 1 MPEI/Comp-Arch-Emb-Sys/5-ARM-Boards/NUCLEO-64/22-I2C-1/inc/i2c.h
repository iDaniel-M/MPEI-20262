

#ifndef __I2C_H__
#define __I2C_H__

#include "system_config.h"

void I2C_clock_enable(I2C_TypeDef* I2C);
void I2C_GPIO_Config(I2C_TypeDef *I2C);

void I2C_Master_Init(I2C_TypeDef* I2C);
void I2C_Master_enable(I2C_TypeDef* I2C, uint8_t val);
void I2C_Master_Start(I2C_TypeDef* I2C);
void I2C_Master_Write(I2C_TypeDef* I2C, uint8_t data);
void I2C_Master_Address(I2C_TypeDef* I2C, uint8_t address);
void I2C_Master_Stop(I2C_TypeDef* I2C);
void I2C_Master_MultiWrite(I2C_TypeDef* I2C, uint8_t *data, uint8_t size);
void I2C_Master_Read(I2C_TypeDef* I2C, uint16_t address, uint8_t* buffer, uint8_t size);


#endif /* __I2C_H__ */