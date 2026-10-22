#include <stdint.h>     // Inclusión de tipos de datos enteros con tamaño definido (uint32_t, uint8_t, etc.)
#include <stdlib.h>     // Inclusión de funciones estándar de gestión de memoria y utilidades generales
#include <string.h>     // Inclusión de funciones para la manipulación de cadenas de texto y bloques de memoria
#include <stdbool.h>    // Inclusión de definiciones para el tipo de dato booleano (true / false)
#include "inc/gpio_config.h"   // Encabezado personalizado para las funciones y estructuras de configuración de GPIO
#include "inc/system_config.h" // Encabezado personalizado para la configuración del sistema (relojes, SysTick, etc.)
#include "inc/gpio_config.h"   // Inclusión duplicada de la configuración de GPIO (mantenida por fidelidad al código original)

#define LED_PIN 5      // Define el pin 5 (PA5) como el pin conectado al LED integrado en la placa NUCLEO-64
#define BUTTON_PIN 13 // Define el pin 13 (PC13) como el pin conectado al botón de usuario integrado en la placa

/* Debe declararse volatile porque la estructura de temporizadores de software se actualiza de forma asíncrona dentro de una interrupción */
volatile SW_Timers timers; // Declaración de la estructura global para el manejo de temporizadores por software

/* El manipulador SysTick_Handler fue declarado como 'weak' durante el archivo de inicialización crt0.s. 
   Al redefinirlo aquí, la tabla de vectores de interrupción se actualiza con la dirección donde se aloja esta función, 
   de modo que cuando ocurre la interrupción SysTick, el microcontrolador sabe a dónde saltar */
void SysTick_Handler(void){
  update_sw_timers(&timers); // Llama a la función que decrementa/actualiza los temporizadores por software en cada tick
}

// Variables globales para almacenar las lecturas del Input Capture (Input Capture Channel 1 y 2)
volatile uint32_t period=0, duty=0; // Variables volátiles para el período (en ciclos) y el tiempo en alto (duty cycle)
float freq=0.0, dutyper=0.0;        // Variables flotantes para almacenar la frecuencia calculada (Hz) y el porcentaje del ciclo de trabajo (%)

// Rutina de Servicio de Interrupción (ISR) para el Temporizador 5 (TIM5)
void TIM5_IRQHandler(void){
  // Verifica si la interrupción fue generada por el evento de captura/comparación en el Canal 1 (CC1IF)
  if(TIM5->SR & TIM_SR_CC1IF){
    period = TIM5->CCR1; // Almacena el valor del registro CCR1 (que representa el período total debido al autoreseteo del temporizador)
    
    // Si el período es válido (diferente de cero), lee el tiempo en alto
    if(period!=0) {
      duty = TIM5->CCR2; // Almacena el valor del registro CCR2 (conteo hasta el flanco de bajada, equivalente al tiempo activo)
    } 
    
    WRITE_REG_FIELD(TIM5->SR, TIM_SR_CC1IF, 0); // Limpia la bandera de interrupción CC1IF escribiendo un 0 en el campo correspondiente del registro de estado (SR)
  }

}

// Inicialización del Temporizador 2 (TIM2) para generar una señal PWM de 10 kHz
void TIM2_PWM_10KHz_Init(void){
  /* Habilita la señal de reloj para el periférico TIM2 en el bus APB1 */
  WRITE_REG_FIELD(RCC->APB1ENR, RCC_APB1ENR_TIM2EN, 1);
  
  // Realiza lecturas 'dummy' al registro APB1ENR para garantizar un retardo de bus antes de acceder a los registros del TIM2
  volatile unsigned int dummy;
  dummy = RCC->APB1ENR; // Primera lectura de retardo
  dummy = RCC->APB1ENR; // Segunda lectura de retardo

  /* Configuración del Timer2 como temporizador base para PWM */
  WRITE_REG(TIM2->PSC, 0);             // Establece el preescalador en 0 (frecuencia de conteo = reloj base del temporizador)
  WRITE_REG(TIM2->ARR, 4200-1);        // Establece el valor de recarga automática (ARR). Para 42 MHz: 42 MHz / 4200 = 10 kHz de frecuencia PWM
  WRITE_REG(TIM2->CCR1, 2100-1);       // Establece el valor del canal 1 (CCR1) para un ciclo de trabajo del 50% (2100 de 4200 conteos)
  WRITE_REG_FIELD(TIM2->CCMR1, TIM_CCMR1_OC1M, 6);  // Configura el modo de salida de comparación en Modo PWM 1 (alto mientras CNT < CCR1)
  WRITE_REG_FIELD(TIM2->CCMR1, TIM_CCMR1_OC1PE, 1); // Habilita la precarga del registro de comparación de salida para el Canal 1 (OC1PE)
  WRITE_REG_FIELD(TIM2->CCER, TIM_CCER_CC1E, 1);    // Habilita la salida física del Canal 1 (CC1E) hacia el pin asignado
  WRITE_REG_FIELD(TIM2->CR1, TIM_CR1_DIR, 0);       // Configura el modo del contador en dirección ascendente (Up-counter)
  WRITE_REG_FIELD(TIM2->CR1, TIM_CR1_ARPE, 1);      // Habilita el búfer de precarga para el registro ARR (Auto-reload preload enable)
  WRITE_REG(TIM2->CNT, 0);               // Reinicia manualmente el contador a 0
  WRITE_REG_FIELD(TIM2->CR1, TIM_CR1_CEN, 1); // Habilita el conteo del temporizador TIM2 (Counter Enable)

}

// Inicialización del Temporizador 5 (TIM5) en modo Captura de Entrada (Input Capture) para medir frecuencia y ciclo de trabajo
void TIM5_IC_Init(void){
  
  /* Habilita la señal de reloj para el periférico TIM5 en el bus APB1 */
  WRITE_REG_FIELD(RCC->APB1ENR, RCC_APB1ENR_TIM5EN, 1);
  
  // Lecturas 'dummy' para garantizar la propagación del reloj de bus hacia TIM5
  volatile unsigned int dummy;
  dummy = RCC->APB1ENR; // Primera lectura de retardo
  dummy = RCC->APB1ENR; // Segunda lectura de retardo

  /* Configuración de TIM5 para captura de señal en alta resolución */
  WRITE_REG(TIM5->PSC, 0);             // Prescale en 0 para contar a la frecuencia máxima del bus (42 MHz)
  WRITE_REG(TIM5->ARR, 0xffffffff);    // Configura el valor máximo de autorecarga (TIM5 es un temporizador de 32 bits)
 
  WRITE_REG_FIELD(TIM5->CCMR1, TIM_CCMR1_CC1S, 1);  // Mapea la entrada del Canal 1 (CC1) a la señal de entrada TI1 (TI1FP1)
  WRITE_REG_FIELD(TIM5->CCER, TIM_CCER_CC1P, 0);   // Configura la captura del Canal 1 en flanco de subida (polarity low = rising edge)
  WRITE_REG_FIELD(TIM5->CCER, TIM_CCER_CC1NP, 0);  // Complemento de polaridad en 0 para confirmar detección en flanco de subida

  WRITE_REG_FIELD(TIM5->CCMR1, TIM_CCMR1_CC2S, 2); // Mapea la entrada del Canal 2 (CC2) a la misma señal física TI1 (TI1FP2)
  WRITE_REG_FIELD(TIM5->CCER, TIM_CCER_CC2P, 1);   // Configura la captura del Canal 2 en flanco de bajada (polarity high = falling edge)
  WRITE_REG_FIELD(TIM5->CCER, TIM_CCER_CC2NP, 0);  // Complemento de polaridad en 0 para confirmar detección en flanco de bajada
  
  WRITE_REG_FIELD(TIM5->SMCR, TIM_SMCR_TS, 5); // Selecciona la señal de disparo (Trigger Selection) como TI1FP1 (filtro de entrada 1)
  WRITE_REG_FIELD(TIM5->SMCR, TIM_SMCR_SMS, 4); // Configura el Modo Esclavo (SMS) en modo 'Reset', reiniciando el contador CNT automáticamente con cada flanco de subida

  WRITE_REG_FIELD(TIM5->CR1, TIM_CR1_DIR, 0); // Configura el conteo en dirección ascendente
  WRITE_REG_FIELD(TIM5->CR1, TIM_CR1_ARPE, 1);    // Habilita la precarga del registro de auto-recarga (ARR)

  WRITE_REG_FIELD(TIM5->CCER, TIM_CCER_CC1E, 1); // Habilita la captura en el Canal 1 (CC1)
  WRITE_REG_FIELD(TIM5->CCER, TIM_CCER_CC2E, 1); // Habilita la captura en el Canal 2 (CC2)
  WRITE_REG_FIELD(TIM5->DIER, TIM_DIER_CC1IE, 1); // Habilita la interrupción por captura/comparación para el Canal 1
  WRITE_REG_FIELD(TIM5->CR1, TIM_CR1_CEN, 1);      // Enciende el temporizador TIM5

  NVIC_EnableIRQ(TIM5_IRQn); // Habilita la línea de interrupción del TIM5 en el controlador vectorial de interrupciones (NVIC) del ARM Cortex-M

}

// Configuración de los pines de E/S (GPIO) de la placa NUCLEO-64
void GPIO_board_config(void){
  GPIO_InitTypeDef GPIO_Init; // Declaración de la estructura de configuración para los pines
  
  // Configuración del pin PA5 (LED de la placa)
  GPIO_Init.Pin = LED_PIN;        // Selecciona el pin 5 del Puerto A (PA5)
  GPIO_Init.Mode = 2;             // Configura el modo del pin en Función Alternativa (Alternate Function)
  GPIO_Init.Pull = 0;             // Sin resistencias de Pull-up ni Pull-down (No pull)
  GPIO_Init.Speed = 3;            // Configura la velocidad de salida en muy alta (Very High Speed)
  GPIO_Init.Alternate = 1;        // Mapea la función alternativa AF1, correspondiente a TIM2_CH1
  GPIO_Config(GPIOA,GPIO_Init);   // Aplica la configuración en el Puerto A (GPIOA)

  // Configuración del pin PA0 (Pin de lectura para Input Capture)
  GPIO_Init.Pin = 0;              // Selecciona el pin 0 del Puerto A (PA0)
  GPIO_Init.Mode = 2;             // Configura el modo del pin en Función Alternativa (Alternate Function)
  GPIO_Init.Pull = 0;             // Sin resistencias de Pull-up ni Pull-down
  GPIO_Init.Alternate = 2;        // Mapea la función alternativa AF2, correspondiente a TIM5_CH1
  GPIO_Config(GPIOA,GPIO_Init);   // Aplica la configuración en el Puerto A (GPIOA)

  // Configuración del pin PC13 (Botón azul integrado en la placa)
  GPIO_Init.Pin = BUTTON_PIN;    // Selecciona el pin 13 del Puerto C (PC13)
  GPIO_Init.Mode = 0;             // Configura el modo del pin como Entrada Digital (Input mode)
  GPIO_Init.Pull = 1;             // Habilita la resistencia interna de Pull-up
  GPIO_Config(GPIOC,GPIO_Init);   // Aplica la configuración en el Puerto C (GPIOC)
}

// Punto de entrada principal del programa
int main()
{

  clock_config();       // Configura las fuentes de reloj del sistema (PLL, HCLK, PCLK1, PCLK2)
  SysTick_Init(1000);   // Configura el temporizador SysTick para generar interrupciones cada 1 ms (1000 Hz)
  SysTick_enable_IrQ(1); // Habilita las interrupciones del SysTick
   
  timers.sw_tmr1_period = 10; // Asigna un período de 10 ticks (10 ms) al temporizador por software 1
  
  GPIO_board_config();   // Configura los pines GPIO utilizados (PA5 para PWM, PA0 para captura, PC13 para botón)
  TIM2_PWM_10KHz_Init(); // Inicializa TIM2 para generar la señal PWM a 10 kHz en el pin PA5
  TIM5_IC_Init();        // Inicializa TIM5 en modo PWM Input Capture en el pin PA0

  // Bucle infinito de ejecución principal
  while(1)
  {
      // Calcula la frecuencia en Hz: (Frecuencia del temporizador 42 MHz) / (conteo total de ciclos por período)
      freq = 42000000.0 / (float)period; 
      
      // Calcula el ciclo de trabajo en porcentaje (%): (conteo de tiempo en alto / conteo total del período) * 100
      dutyper = ((float) duty / (float) period)*100;
  }

}