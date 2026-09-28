#ifndef _SPI_H_
#define _SPI_H_

#include "system_config.h"


void SPI_GPIO_Config(SPI_TypeDef *SPI);

void SPI_Slave_IRQ(SPI_TypeDef *SPI,uint8_t val);

void SPI_Master_Init(SPI_TypeDef * SPI);
void SPI_Slave_Init(SPI_TypeDef *SPI);

void SPI_Enable(SPI_TypeDef *SPI,char enable);

char SPI_Master_Tranfer(SPI_TypeDef *SPI, uint8_t  *data, uint32_t size);

void SPI_Slave_Transmit(SPI_TypeDef* SPI,uint8_t txdata);

uint8_t SPI_Slave_Receive(SPI_TypeDef* SPI);


#endif