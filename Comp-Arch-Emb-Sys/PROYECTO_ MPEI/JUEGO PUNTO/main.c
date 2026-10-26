#include <stdio.h>
#include <stdint.h>
#include "inc/gpio_config.h"
#include "inc/system_config.h"
#include "inc/usart.h"
#include "inc/p10.h"

/*
 * JUEGO PUNTO - Un píxel que se mueve por el panel P10 (32x16).
 * Basado en el ejercicio 21-SPI-4 del profesor (el max7219 se cambió por el panel P10).
 *
 * Cada 200 ms (sw_tmr1) el punto avanza un paso en la dirección actual, sin salirse
 * del panel, y se imprime x, y por la UART (115200 baudios).
 * La dirección se elige con los botones del shield o con teclas por la UART:
 *   botón A (PA10) / 'w' = arriba      botón B (PB3)  / 'd' = derecha
 *   botón C (PB5)  / 's' = abajo       botón D (PB4)  / 'a' = izquierda
 *   botón E (PB10) / 'p' = pausa       botón F (PA8)  / 'r' = reiniciar (punto al centro)
 */

#define LED_PIN 5        // PA5 = LED verde LD2 de la Nucleo (parpadea con cada paso)

#define BTN_A_PIN 10     // PA10 arriba
#define BTN_B_PIN 3      // PB3  derecha
#define BTN_C_PIN 5      // PB5  abajo
#define BTN_D_PIN 4      // PB4  izquierda
#define BTN_E_PIN 10     // PB10 pausa
#define BTN_F_PIN 8      // PA8  reiniciar

/* Must be delcared volatile as the timer can update asynchounosly*/
volatile SW_Timers timers;
volatile uint8_t uart_rx_flag=0;
volatile uint8_t uart_rx_data=0;


/*The SysTick_Handler, was already defined as weak during the crt0.s init file, so when we define it here,
the Vector table is updated whit the new address where the function is allocated, so that when and interrupt happen
the vector table knows where to find the SysTick_Handler*/
void SysTick_Handler(void){
  update_sw_timers(&timers);
}


void USART2_IRQHandler(void){
  if(USART2->SR&USART_SR_RXNE){
    uart_rx_data = uart_getc(USART2);
    uart_rx_flag = 1;
  }
}


/* Configura un botón: entrada (Mode 0) con pull-up (Pull 1).
   Los botones del shield conectan el pin a GND al presionar: suelto = 1, presionado = 0. */
void button_config(volatile GPIO_TypeDef *GPIOx, uint32_t pin){
  GPIO_InitTypeDef GPIO_Init = {0};
  GPIO_Init.Pin = pin;
  GPIO_Init.Mode = 0;   // entrada. En PB3 y PB4 esto es obligatorio: al encender el micro
                        // los deja en función alterna (PB3 = JTDO/SWO, PB4 = NJTRST del JTAG)
  GPIO_Init.Pull = 1;   // 01 = pull-up interno
  GPIO_Config(GPIOx, GPIO_Init);
}

void GPIO_board_config(void){
  GPIO_InitTypeDef GPIO_Init = {0};
  GPIO_Init.Pin = LED_PIN;
  GPIO_Init.Mode = 1; // Output function mode
  GPIO_Init.Pull = 0; // No pull-up or pull-down
  GPIO_Init.Speed = 3;
  GPIO_Config(GPIOA,GPIO_Init);

  button_config(GPIOA, BTN_A_PIN);
  button_config(GPIOB, BTN_B_PIN);
  button_config(GPIOB, BTN_C_PIN);
  button_config(GPIOB, BTN_D_PIN);
  button_config(GPIOB, BTN_E_PIN);
  button_config(GPIOA, BTN_F_PIN);
}

typedef struct
{
  uint8_t x;
  uint8_t y;
} pointd;

pointd point;


/* Borra la imagen y enciende solo el píxel (x, y) */
void draw_point(pointd point){
  p10_clear();
  p10_set_pixel(point.x, point.y, P10_ROJO);
}

/* Lee los botones (activos en bajo: presionado = 0) y devuelve la tecla equivalente,
   o 0 si no hay ninguno presionado. */
char read_buttons(void){
  if(read_pin_state(GPIOA, BTN_A_PIN) == 0) return 'w';
  if(read_pin_state(GPIOB, BTN_B_PIN) == 0) return 'd';
  if(read_pin_state(GPIOB, BTN_C_PIN) == 0) return 's';
  if(read_pin_state(GPIOB, BTN_D_PIN) == 0) return 'a';
  if(read_pin_state(GPIOB, BTN_E_PIN) == 0) return 'p';
  if(read_pin_state(GPIOA, BTN_F_PIN) == 0) return 'r';
  return 0;
}

int main()
{

  clock_config();       // 84 MHz con el PLL
  SysTick_Init(1000);   // SysTick cada 1000 us = 1 ms (base de los sw timers)
  SysTick_enable_IrQ(1);

  timers.sw_tmr1_period = 200;   // el punto avanza cada 200 ms
  timers.sw_tmr2_period = 1000;

  GPIO_board_config();

  UART_GPIO_Config(USART2);
  UART_Init(USART2,115200);
  UART_RX_IRQ_EN(USART2,1);
  NVIC_EnableIRQ(USART2_IRQn);

  p10_config();

  char p_message[20];
  point.x = P10_ANCHO / 2;   // empezar en el centro (16, 8)
  point.y = P10_ALTO / 2;

  draw_point(point);
  char direction = 'p';      // empieza quieto
  while(1)
  {
    if(timers.sw_tmr1_flag){
      timers.sw_tmr1_flag = 0;
      toggle_pin_state(GPIOA,LED_PIN);
      // y = 0 es la fila de ARRIBA del panel: subir es restar 1 a y
      if(direction=='w'){
        if(point.y>0){
          point.y--;
        }
      } else if (direction=='s') {
        if(point.y<P10_ALTO-1){
          point.y++;
        }
      } else if (direction=='a') {
        if(point.x>0){
          point.x--;
        }
      }else if (direction=='d') {
        if(point.x<P10_ANCHO-1){
          point.x++;
        }
      }
      sprintf(p_message,"x=%d,y=%d\n\r",point.x,point.y);
      uart_puts(USART2,p_message);
      draw_point(point);
    }

    // Tecla nueva: de un botón o de la UART
    char key = read_buttons();
    if(uart_rx_flag){
      uart_rx_flag = 0;
      key = uart_rx_data;
    }

    if(key == 'r'){               // reiniciar: punto al centro y quieto
      point.x = P10_ANCHO / 2;
      point.y = P10_ALTO / 2;
      direction = 'p';
      draw_point(point);
    } else if(key == 'w' || key == 'a' || key == 's' || key == 'd' || key == 'p'){
      direction = key;            // 'p' = pausa: ninguna dirección se mueve
    }
  }


}
