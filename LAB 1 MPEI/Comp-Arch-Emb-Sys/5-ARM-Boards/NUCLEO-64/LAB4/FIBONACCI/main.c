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
