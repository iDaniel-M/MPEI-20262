/*************************************************************************************
 * PROYECTO: Conversor Binario a BCD (Lectura de DIP Switch a Display 7-Segmentos)
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
 *    * Nota: El otro extremo de todos los interruptores va a Tierra (GND).
 *
 * 2. SALIDAS: SEGMENTOS DEL DISPLAY (PA0, PA1, PA4, PA5, PA6, PA7, PA8)
 *    Lógica: Escribir un '0' enciende el segmento (Ánodo Común / GND activo).
 *    - PA0, PA1, PA4, PA5, PA6, PA7, PA8 -> Conectados a los segmentos A-G.
 *
 * 3. SALIDAS: MULTIPLEXADO (Transistores de los Dígitos)
 *    Lógica: Escribir un '1' habilita el display correspondiente.
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

void Default_Handler(void) {
    while(1);
}

__attribute__((section(".isr_vector"), used))
void *vector_table[] = {
    (void *)&_estack,        // 0:  Stack pointer inicial
    (void *)Reset_Handler,   // 1:  Reset
    (void *)Default_Handler, // 2:  NMI
    (void *)Default_Handler, // 3:  HardFault
    (void *)Default_Handler, // 4:  MemManage
    (void *)Default_Handler, // 5:  BusFault
    (void *)Default_Handler, // 6:  UsageFault
    0, 0, 0, 0,               // 7-10: reservado
    (void *)Default_Handler, // 11: SVCall
    (void *)Default_Handler, // 12: DebugMon
    0,                        // 13: reservado
    (void *)Default_Handler, // 14: PendSV
    (void *)Default_Handler, // 15: SysTick
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
#define GPIOC_MODER           (*(volatile uint32_t *)(GPIOC_BASE + 0x00U))
#define GPIOC_PUPDR           (*(volatile uint32_t *)(GPIOC_BASE + 0x0CU))
#define GPIOC_IDR             (*(volatile uint32_t *)(GPIOC_BASE + 0x10U))

// Variables requeridas para almacenar el resultado BCD
volatile uint8_t centenas = 0;
volatile uint8_t tens = 0;
volatile uint8_t units = 0;

void delay(void) {
    volatile uint32_t i;
    for (i = 0; i < 3000; i++) { ; }
}

void display_off(void) {
    GPIOB_ODR &= ~(1 << 0); // PB0 (D1 - Centenas)
    GPIOB_ODR &= ~(1 << 5); // PB5 (D2 - Decenas)
    GPIOB_ODR &= ~(1 << 2); // PB2 (D3 - Unidades)
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

void display_number(void) {
    display_off();
    display_segments(units);
    GPIOB_ODR |= (1 << 2); // Activa Unidades (PB2)
    delay();

    display_off();
    display_segments(tens);
    GPIOB_ODR |= (1 << 5); // Activa Decenas (PB5)
    delay();

    display_off();
    display_segments(centenas);
    GPIOB_ODR |= (1 << 0); // Activa Centenas (PB0)
    delay();
}

void GPIO_Config(void) {
    // Habilitar relojes para GPIOA, GPIOB y GPIOC
    RCC_AHB1ENR |= (1 << 0);
    RCC_AHB1ENR |= (1 << 1);
    RCC_AHB1ENR |= (1 << 2);
    (void)RCC_AHB1ENR; // sincroniza el reloj antes de tocar los registros del periférico

    // Configurar segmentos PA0, PA1, PA4, PA5, PA6, PA7, PA8 como salidas
    GPIOA_MODER &= ~(3 << (0 * 2)); GPIOA_MODER |=  (1 << (0 * 2));
    GPIOA_MODER &= ~(3 << (1 * 2)); GPIOA_MODER |=  (1 << (1 * 2));
    GPIOA_MODER &= ~(3 << (4 * 2)); GPIOA_MODER |=  (1 << (4 * 2));
    GPIOA_MODER &= ~(3 << (5 * 2)); GPIOA_MODER |=  (1 << (5 * 2));
    GPIOA_MODER &= ~(3 << (6 * 2)); GPIOA_MODER |=  (1 << (6 * 2));
    GPIOA_MODER &= ~(3 << (7 * 2)); GPIOA_MODER |=  (1 << (7 * 2));
    GPIOA_MODER &= ~(3 << (8 * 2)); GPIOA_MODER |=  (1 << (8 * 2));

    // Configurar pines de dígitos PB0, PB5, PB2 como salidas
    GPIOB_MODER &= ~(3 << (0 * 2)); GPIOB_MODER |=  (1 << (0 * 2));
    GPIOB_MODER &= ~(3 << (5 * 2)); GPIOB_MODER |=  (1 << (5 * 2));
    GPIOB_MODER &= ~(3 << (2 * 2)); GPIOB_MODER |=  (1 << (2 * 2));

    // Configurar pines PC0-PC7 (DIP Switch) como entradas con pull-up interno
    GPIOC_MODER &= 0xFFFF0000; // Entradas en PC0-PC7
    GPIOC_PUPDR &= 0xFFFF0000;
    GPIOC_PUPDR |= 0x00005555; // Pull-up en PC0-PC7

    display_off();
}

int main(void) {
    uint16_t raw_input;

    GPIO_Config();

    while (1) {
        // Lee el puerto C (PC0-PC7), invierte los bits por la lógica activa-low de los switches
        raw_input = (~GPIOC_IDR) & 0xFF;

        // Conversión a BCD y almacenamiento en las posiciones de memoria solicitadas
        centenas = raw_input / 100;
        tens = (raw_input - (centenas * 100)) / 10;
        units = raw_input - (centenas * 100) - (tens * 10);

        // Manda a refrescar el display con los valores convertidos
        display_number();
    }
}