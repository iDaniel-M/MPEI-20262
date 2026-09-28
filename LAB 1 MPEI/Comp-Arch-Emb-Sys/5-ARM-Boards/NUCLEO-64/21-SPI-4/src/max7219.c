#include "max7219.h"
#include "spi.h"
#include "gpio_config.h"

uint8_t init_data[5][2] = {
    {0x9,0x00},
    {0xA,0x01},
    {0xB,0x07},
    {0xC,0x01},
    {0xF,0x00}
};




void max7219_config(void){
    SPI_GPIO_Config(SPI2);
    SPI_Master_Init(SPI2);
    SPI_Enable(SPI2,1);
    write_pin_state(GPIOB,9,1);
    
    for(int8_t i=0; i<5; i++){
        write_pin_state(GPIOB,9,0);
        SPI_Master_Tranfer(SPI2, init_data[i],2);
        write_pin_state(GPIOB,9,1);
    }

}


void max7219_print(uint8_t * Matrix){
    uint8_t row[2];
    for(uint8_t i=0; i<8; i++){
        row[0] = i+1;
        row[1] = Matrix[i];
        write_pin_state(GPIOB,9,0);
        SPI_Master_Tranfer(SPI2, row,2);
        write_pin_state(GPIOB,9,1);
    }
}