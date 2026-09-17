/*************************************************************************************
 * PROYECTO: Control de Displays 7-Segmentos y Lectura de DIP Switch (Bare-Metal)
 * TARJETA:  STM32 Nucleo-64 (Ej. NUCLEO-F401RE / F411RE - ARM Cortex-M4)
 * 
 * --- MAPA DE CONEXIONES (PINOUT) ---
 *
 * 1. ENTRADAS: DIP SWITCH (8 Bits)
 *    Lógica: Pull-up interno activado. Un pin a GND se lee como 1 (Activo en Bajo).
 *    - PC0 -> SW 1 (Bit 0 - LSB)  [Físico: CN8 Pin 6 / CN7 Pin 38]
 *    - PC1 -> SW 2 (Bit 1)        [Físico: CN8 Pin 5 / CN7 Pin 36]
 *    - PC2 -> SW 3 (Bit 2)        [Físico: CN7 Pin 35]
 *    - PC3 -> SW 4 (Bit 3)        [Físico: CN7 Pin 37]
 *    - PC4 -> SW 5 (Bit 4)        [Físico: CN10 Pin 34]
 *    - PC5 -> SW 6 (Bit 5)        [Físico: CN10 Pin 6]
 *    - PC6 -> SW 7 (Bit 6)        [Físico: CN10 Pin 4]
 *    - PC7 -> SW 8 (Bit 7 - MSB)  [Físico: CN5 Pin 2 (D9) / CN10 Pin 19]
 *    * Nota: El otro lado de todos los interruptores del DIP Switch va a Tierra (GND).
 *
 * 2. SALIDAS: SEGMENTOS DEL DISPLAY (PA0, PA1, PA4, PA5, PA6, PA7, PA8)
 *    Lógica: Escribir un '0' enciende el segmento (Ánodo Común / GND activo).
 *    - PA0, PA1, PA4, PA5, PA6, PA7, PA8 -> Conectados a los segmentos A-G a través de
 *                                           resistencias limitadoras.
 *
 * 3. SALIDAS: MULTIPLEXADO (Transistores de los Dígitos)
 *    - PB0 -> Habilita Display 1 (Centenas)
 *    - PB5 -> Habilita Display 2 (Decenas)
 *    - PB2 -> Habilita Display 3 (Unidades)
 *************************************************************************************/

#include <stdint.h>

extern uint32_t _estack;
extern int main(void);

void Reset_Handler(void) {
    main();
    while(1);
}

__attribute__((section(".isr_vector"), used))
void *vector_table[] = {
    (void *)&_estack,
    (void *)Reset_Handler
};

#define PERIPHERAL_BASE       (0x40000000U)
#define AHB1_BASE             (PERIPHERAL_BASE + 0x00020000U)
#define GPIOA_BASE            (AHB1_BASE + 0x0000U)
#define GPIOB_BASE            (AHB1_BASE + 0x0400U)
#define GPIOC_BASE            (AHB1_BASE + 0x0800U)
#define RCC_BASE              (AHB1_BASE + 0x3800U)

#define RCC_AHB1ENR           (*(volatile uint32_t *)(RCC_BASE + 0x30U))
#define GPIOA_MODER           (*(volatile uint32_t *)(GPIOA_BASE + 0x00U))
#define GPIOA_ODR             (*(volatile uint32_t *)(GPIOA_BASE + 0x14U))
#define GPIOB_MODER           (*(volatile uint32_t *)(GPIOB_BASE + 0x00U))
#define GPIOB_ODR             (*(volatile uint32_t *)(GPIOB_BASE + 0x14U))

void delay(void) {
    volatile uint32_t i;
    for (i = 0; i < 3000; i++) { ; }
}

void display_off(void) {
    GPIOB_ODR &= ~(1 << 0);
    GPIOB_ODR &= ~(1 << 5);
    GPIOB_ODR &= ~(1 << 2);
}

void display_segments(uint8_t numero) {
   GPIOA_ODR |= (1 << 0); GPIOA_ODR |= (1 << 1); GPIOA_ODR |= (1 << 7);
   GPIOA_ODR |= (1 << 8); GPIOA_ODR |= (1 << 4); GPIOA_ODR |= (1 << 5);
   GPIOA_ODR |= (1 << 6);

    if (numero == 0) {
      GPIOA_ODR &= ~(1 << 0); GPIOA_ODR &= ~(1 << 1); GPIOA_ODR &= ~(1 << 7);
      GPIOA_ODR &= ~(1 << 8); GPIOA_ODR &= ~(1 << 4); GPIOA_ODR &= ~(1 << 5);
    }
    if (numero == 1) { GPIOA_ODR &= ~(1 << 1); GPIOA_ODR &= ~(1 << 7); }
    if (numero == 2) {
      GPIOA_ODR &= ~(1 << 0); GPIOA_ODR &= ~(1 << 1); GPIOA_ODR &= ~(1 << 8);
      GPIOA_ODR &= ~(1 << 4); GPIOA_ODR &= ~(1 << 6);
    }
    if (numero == 3) {
      GPIOA_ODR &= ~(1 << 0); GPIOA_ODR &= ~(1 << 1); GPIOA_ODR &= ~(1 << 7);
      GPIOA_ODR &= ~(1 << 8); GPIOA_ODR &= ~(1 << 6);
    }
    if (numero == 4) {
      GPIOA_ODR &= ~(1 << 1); GPIOA_ODR &= ~(1 << 7);
      GPIOA_ODR &= ~(1 << 5); GPIOA_ODR &= ~(1 << 6);
    }
    if (numero == 5) {
      GPIOA_ODR &= ~(1 << 0); GPIOA_ODR &= ~(1 << 7); GPIOA_ODR &= ~(1 << 8);
      GPIOA_ODR &= ~(1 << 5); GPIOA_ODR &= ~(1 << 6);
    }
    if (numero == 6) {
      GPIOA_ODR &= ~(1 << 0); GPIOA_ODR &= ~(1 << 7); GPIOA_ODR &= ~(1 << 8);
      GPIOA_ODR &= ~(1 << 4); GPIOA_ODR &= ~(1 << 5); GPIOA_ODR &= ~(1 << 6);
    }
    if (numero == 7) {
      GPIOA_ODR &= ~(1 << 0); GPIOA_ODR &= ~(1 << 1); GPIOA_ODR &= ~(1 << 7);
    }
    if (numero == 8) {
      GPIOA_ODR &= ~(1 << 0); GPIOA_ODR &= ~(1 << 1); GPIOA_ODR &= ~(1 << 7);
      GPIOA_ODR &= ~(1 << 8); GPIOA_ODR &= ~(1 << 4); GPIOA_ODR &= ~(1 << 5);
      GPIOA_ODR &= ~(1 << 6);
    }
    if (numero == 9) {
      GPIOA_ODR &= ~(1 << 0); GPIOA_ODR &= ~(1 << 1); GPIOA_ODR &= ~(1 << 7);
      GPIOA_ODR &= ~(1 << 8); GPIOA_ODR &= ~(1 << 5); GPIOA_ODR &= ~(1 << 6);
    }
}

void display_number(uint16_t numero) {
    uint8_t centenas = numero / 100;
    uint8_t decenas = (numero - (centenas * 100)) / 10;
    uint8_t unidades = numero - (centenas * 100) - (decenas * 10);

    display_off();
    display_segments(unidades);
    GPIOB_ODR |= (1 << 2);
    delay();

    display_off();
    display_segments(decenas);
    GPIOB_ODR |= (1 << 5);
    delay();

    display_off();
    display_segments(centenas);
    GPIOB_ODR |= (1 << 0);
    delay();
}

void GPIO_Config(void) {
    RCC_AHB1ENR |= (1 << 0);
    RCC_AHB1ENR |= (1 << 1);
    RCC_AHB1ENR |= (1 << 2);

    GPIOA_MODER &= ~(3 << (0 * 2)); GPIOA_MODER |=  (1 << (0 * 2));
    GPIOA_MODER &= ~(3 << (1 * 2)); GPIOA_MODER |=  (1 << (1 * 2));
    GPIOA_MODER &= ~(3 << (4 * 2)); GPIOA_MODER |=  (1 << (4 * 2));
    GPIOA_MODER &= ~(3 << (5 * 2)); GPIOA_MODER |=  (1 << (5 * 2));
    GPIOA_MODER &= ~(3 << (6 * 2)); GPIOA_MODER |=  (1 << (6 * 2));
    GPIOA_MODER &= ~(3 << (7 * 2)); GPIOA_MODER |=  (1 << (7 * 2));
    GPIOA_MODER &= ~(3 << (8 * 2)); GPIOA_MODER |=  (1 << (8 * 2));

    GPIOB_MODER &= ~(3 << (0 * 2)); GPIOB_MODER |=  (1 << (0 * 2));
    GPIOB_MODER &= ~(3 << (5 * 2)); GPIOB_MODER |=  (1 << (5 * 2));
    GPIOB_MODER &= ~(3 << (2 * 2)); GPIOB_MODER |=  (1 << (2 * 2));

    display_off();

    GPIOA_ODR |= (1 << 0); GPIOA_ODR |= (1 << 1); GPIOA_ODR |= (1 << 4);
    GPIOA_ODR |= (1 << 5); GPIOA_ODR |= (1 << 6); GPIOA_ODR |= (1 << 7);
    GPIOA_ODR |= (1 << 8);
}

int main(void) {
    uint16_t a = 0;
    uint16_t b = 1;
    uint16_t next_fib = 0;
    uint16_t j;

    GPIO_Config();

    while (1) {
        // Mantiene el número visible durante aprox. 250ms mediante bucles de refresco
        for (j = 0; j < 40; j++) {
            display_number(a);
        }

        // Siguiente valor de Fibonacci
        next_fib = a + b;
        a = b;
        b = next_fib;

        // Limita la secuencia hasta el rango permitido por los displays (máximo 255/233)
        if (a > 255) {
            a = 0;
            b = 1;
        }
    }
}
