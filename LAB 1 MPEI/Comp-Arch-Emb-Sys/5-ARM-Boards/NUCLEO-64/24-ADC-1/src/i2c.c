


#include "i2c.h"

void I2C_clock_enable(I2C_TypeDef* I2C){
    if(I2C==I2C1){
        WRITE_REG_FIELD(RCC->APB1ENR,RCC_APB1ENR_I2C1EN,1);
    } else if(I2C==I2C2){
        WRITE_REG_FIELD(RCC->APB1ENR,RCC_APB1ENR_I2C2EN,1);
    } else if(I2C==I2C3){
        WRITE_REG_FIELD(RCC->APB1ENR,RCC_APB1ENR_I2C3EN,1);
    }
    volatile unsigned int dummy;
    dummy =  RCC->APB1ENR;
    dummy =  RCC->APB1ENR;
}

void I2C_GPIO_Config(I2C_TypeDef *I2C){
    if(I2C==I2C1){
        I2C_clock_enable(I2C);
        WRITE_REG_FIELD(RCC->AHB1ENR, RCC_AHB1ENR_GPIOBEN, 1);
        // do two dummy reads after enabling the peripheral clock, as per the errata
        volatile unsigned int dummy;
        dummy = (RCC->AHB1ENR);
        dummy = (RCC->AHB1ENR);

        /*I2C1 Config Pins PB6->SCL, PB7->SDA*/
        WRITE_REG_FIELD(GPIOB->MODER,GPIO_MODER_MODER6,2);
        WRITE_REG_FIELD(GPIOB->AFR[0],GPIO_AFRL_AFRL6,4);
        WRITE_REG_FIELD(GPIOB->OTYPER,GPIO_OTYPER_OT6,1); // Open Drain 

        WRITE_REG_FIELD(GPIOB->MODER,GPIO_MODER_MODER7,2);
        WRITE_REG_FIELD(GPIOB->AFR[0],GPIO_AFRL_AFRL7,4);
        WRITE_REG_FIELD(GPIOB->OTYPER,GPIO_OTYPER_OT7,1); // Open Drain
    } 
    if(I2C==I2C2){
        //TODO
    }
    if(I2C==I2C3){
        //TODO
    }

}


void I2C_Master_Init(I2C_TypeDef* I2C){
    WRITE_REG_FIELD(I2C->CR1, I2C_CR1_SWRST, 1);
    WRITE_REG_FIELD(I2C->CR1, I2C_CR1_SWRST, 0);
    WRITE_REG_FIELD(I2C->CR2, I2C_CR2_FREQ, 42);
    I2C->CCR = 210;
    I2C->TRISE = 43;
}

void I2C_Master_enable(I2C_TypeDef* I2C, uint8_t val){
    WRITE_REG_FIELD(I2C->CR1,I2C_CR1_PE,val);
}


void I2C_Master_Start(I2C_TypeDef* I2C){
    SET_BIT(I2C->CR1,I2C_CR1_ACK);
    SET_BIT(I2C->CR1,I2C_CR1_START);
    while(!READ_REG_FIELD(I2C->SR1,I2C_SR1_SB));
}


void I2C_Master_Write(I2C_TypeDef* I2C, uint8_t data){
    while(!READ_REG_FIELD(I2C->SR1,I2C_SR1_TXE));
    I2C->DR = data;
    while(!READ_REG_FIELD(I2C->SR1,I2C_SR1_BTF));
}


void I2C_Master_Address(I2C_TypeDef* I2C, uint8_t address){
    I2C->DR = address;
    while(!READ_REG_FIELD(I2C->SR1,I2C_SR1_ADDR));
    uint8_t tmp = I2C->SR1 | I2C->SR2;
}

void I2C_Master_Stop(I2C_TypeDef* I2C){
    SET_BIT(I2C->CR1,I2C_CR1_STOP);
}

void I2C_Master_MultiWrite(I2C_TypeDef* I2C, uint8_t *data, uint8_t size){
    while(!READ_REG_FIELD(I2C->SR1,I2C_SR1_TXE));
    while(size){
        while(!READ_REG_FIELD(I2C->SR1,I2C_SR1_TXE));
        I2C->DR = ( uint32_t)*data++;
        size--;
    }
    while(!READ_REG_FIELD(I2C->SR1,I2C_SR1_BTF));
}

void I2C_Master_Read(I2C_TypeDef* I2C, uint16_t address, uint8_t* buffer, uint8_t size){
    int remaining = size;
    if(size==1){
        I2C->DR = address;
        while(!(READ_REG_FIELD(I2C->SR1,I2C_SR1_ADDR)));
        CLEAR_BIT(I2C->CR1,I2C_CR1_ACK);

        volatile uint8_t tmp = I2C->SR1 | I2C->SR2;
        SET_BIT(I2C->CR1,I2C_CR1_STOP);
        while(!(READ_REG_FIELD(I2C->SR1,I2C_SR1_RXNE)));
        buffer[size-remaining] = I2C->DR;
    } else {
        I2C->DR = address;
        while(!(READ_REG_FIELD(I2C->SR1,I2C_SR1_ADDR)));
        volatile uint8_t tmp = I2C->SR1 | I2C->SR2;
        while(remaining>2){
            while(!(READ_REG_FIELD(I2C->SR1,I2C_SR1_RXNE)));
            buffer[size-remaining] = I2C->DR;
            SET_BIT(I2C->CR1,I2C_CR1_ACK);
            remaining--;
        }
        while(!(READ_REG_FIELD(I2C->SR1,I2C_SR1_RXNE)));
        buffer[size-remaining] = I2C->DR;
        CLEAR_BIT(I2C->CR1,I2C_CR1_ACK);
        SET_BIT(I2C->CR1,I2C_CR1_STOP);
        remaining--;
        while(!(READ_REG_FIELD(I2C->SR1,I2C_SR1_RXNE)));
        buffer[size-remaining] = I2C->DR;
    }
}