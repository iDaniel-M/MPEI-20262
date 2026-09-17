#include "stm32f446xx.h"
#include <stdint.h>

// ================================================================
// CONFIGURACIÓN DEL SISTEMA
// ================================================================

#define TIMER_CLOCK_HZ       16000000UL 
#define TIM3_PSC_VAL         16       

#define MIN_FREQUENCY        1000UL
#define MAX_FREQUENCY        99999UL  // Ampliado para permitir medir hasta casi 100kHz
#define DISPLAY_DIGITS       5U

#define SEGMENTS_ACTIVE_LOW  1
#define COMMONS_ACTIVE_LOW   0 // Transistores NPN: 1 enciende, 0 apaga

// ================================================================
// PROTOTIPOS
// ================================================================
void GPIO_Init(void);
void TIM2_IC_Init(void);
void TIM3_Multiplex_Init(void);
void Update_Display_Digits(uint32_t freq);
void TIM2_IRQHandler(void);
void TIM3_IRQHandler(void);
int main(void);

// ================================================================
// TABLA DE VECTORES
// ================================================================
extern uint32_t _estack;

void Reset_Handler(void) { main(); while (1); }
void Default_Handler(void) { while (1); }

__attribute__((section(".isr_vector")))
void (* const g_pfnVectors[])(void) =
{
    (void (*)(void))(&_estack),
    Reset_Handler,
    Default_Handler, Default_Handler, Default_Handler, Default_Handler, 
    Default_Handler, Default_Handler, 0, 0, 0, 0,                       
    Default_Handler, Default_Handler, 0, Default_Handler, Default_Handler, 
    [16 + 28] = TIM2_IRQHandler, 
    [16 + 29] = TIM3_IRQHandler, 
};

// ================================================================
// VARIABLES GLOBALES
// ================================================================
volatile uint8_t current_digit = 0;
volatile uint8_t digits_to_display[DISPLAY_DIGITS] = {0, 0, 0, 0, 0};

// Variables para el cálculo del promedio cada 1 segundo
// IMPORTANTE: freq_sum ahora es de 64 bits para evitar desbordamiento a altas frecuencias
volatile uint64_t freq_sum = 0;       
volatile uint32_t freq_count = 0;     
volatile uint32_t display_freq = 0;   
volatile uint8_t one_second_flag = 0; 

volatile uint32_t last_capture_tick = 0;

#if SEGMENTS_ACTIVE_LOW
    #define SEG_OFF(pin) (1UL << (pin))
    #define SEG_ON(pin)  (1UL << ((pin) + 16))
#else
    #define SEG_OFF(pin) (1UL << ((pin) + 16))
    #define SEG_ON(pin)  (1UL << (pin))
#endif

#if COMMONS_ACTIVE_LOW
    #define COM_OFF(pin) (1UL << (pin))
    #define COM_ON(pin)  (1UL << ((pin) + 16))
#else
    #define COM_OFF(pin) (1UL << ((pin) + 16)) 
    #define COM_ON(pin)  (1UL << (pin))        
#endif

const uint8_t segment_map[10] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

const uint32_t segment_mask_complete[10] = {
    // 0: A,B,C,D,E,F ON
    SEG_ON(0) | SEG_ON(7) | SEG_ON(8) | SEG_ON(3) | SEG_ON(4) | SEG_ON(5),
    // 1: B,C ON | A,D,E,F OFF
    SEG_OFF(0) | SEG_ON(7) | SEG_ON(8) | SEG_OFF(3) | SEG_OFF(4) | SEG_OFF(5),
    // 2: A,B,D,E ON | C,F OFF
    SEG_ON(0) | SEG_ON(7) | SEG_OFF(8) | SEG_ON(3) | SEG_ON(4) | SEG_OFF(5),
    // 3: A,B,C,D ON | E,F OFF
    SEG_ON(0) | SEG_ON(7) | SEG_ON(8) | SEG_ON(3) | SEG_OFF(4) | SEG_OFF(5),
    // 4: B,C,F ON | A,D,E OFF
    SEG_OFF(0) | SEG_ON(7) | SEG_ON(8) | SEG_OFF(3) | SEG_OFF(4) | SEG_ON(5),
    // 5: A,C,D,F ON | B,E OFF
    SEG_ON(0) | SEG_OFF(7) | SEG_ON(8) | SEG_ON(3) | SEG_OFF(4) | SEG_ON(5),
    // 6: A,C,D,E,F ON | B OFF
    SEG_ON(0) | SEG_OFF(7) | SEG_ON(8) | SEG_ON(3) | SEG_ON(4) | SEG_ON(5),
    // 7: A,B,C ON | D,E,F OFF
    SEG_ON(0) | SEG_ON(7) | SEG_ON(8) | SEG_OFF(3) | SEG_OFF(4) | SEG_OFF(5),
    // 8: A,B,C,D,E,F ON
    SEG_ON(0) | SEG_ON(7) | SEG_ON(8) | SEG_ON(3) | SEG_ON(4) | SEG_ON(5),
    // 9: A,B,C,D,F ON | E OFF
    SEG_ON(0) | SEG_ON(7) | SEG_ON(8) | SEG_ON(3) | SEG_OFF(4) | SEG_ON(5)
};

// ================================================================
// CONFIGURACIÓN DE GPIO
// ================================================================
void GPIO_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN;

    // PA0: TIM2_CH1 (Input Capture)
    GPIOA->MODER &= ~(3UL << 0);
    GPIOA->MODER |= (2UL << 0); 
    GPIOA->AFR[0] &= ~(0xFUL << 0);
    GPIOA->AFR[0] |= (1UL << 0); 
    GPIOA->PUPDR &= ~(3UL << 0);
    GPIOA->PUPDR |= (2UL << 0); 

    // PC0, PC3, PC4, PC5, PC7, PC8: Segmentos (Salida)
    uint32_t mask_c = (3UL << 0) | (3UL << 6) | (3UL << 8) | (3UL << 10) | (3UL << 14) | (3UL << 16);
    GPIOC->MODER &= ~mask_c;
    GPIOC->MODER |= (1UL << 0) | (1UL << 6) | (1UL << 8) | (1UL << 10) | (1UL << 14) | (1UL << 16);

    // PA9: Segmento G (Salida)
    GPIOA->MODER &= ~(3UL << 18);
    GPIOA->MODER |= (1UL << 18);

    // PB0-PB4: Commons (Salida)
    GPIOB->MODER &= ~0x000003FF;
    GPIOB->MODER |= 0x00000155;

    // Estado inicial: Todo apagado
    GPIOB->BSRR = COM_OFF(0) | COM_OFF(1) | COM_OFF(2) | COM_OFF(3) | COM_OFF(4);
    GPIOC->BSRR = SEG_OFF(0) | SEG_OFF(3) | SEG_OFF(4) | SEG_OFF(5) | SEG_OFF(7) | SEG_OFF(8);
    GPIOA->BSRR = SEG_OFF(9);
}

// ================================================================
// CONFIGURACIÓN TIM2 (INPUT CAPTURE)
// ================================================================
void TIM2_IC_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->PSC = 0;          
    TIM2->ARR = 0xFFFFFFFF; 

    TIM2->CCMR1 &= ~(3UL << 0);
    TIM2->CCMR1 |= (1UL << 0); // CC1S = 01 (TI1)

    TIM2->CCMR1 &= ~(0xFUL << 4);
    TIM2->CCMR1 |= (4UL << 4); // IC1F = 0100 (filtro de hardware)

    TIM2->CCER &= ~((1UL << 1) | (1UL << 3));
    TIM2->CCER |= (1UL << 0);  // CC1E = 1

    TIM2->DIER |= TIM_DIER_CC1IE;
    NVIC_EnableIRQ(28);
    
    TIM2->CNT = 0;
    TIM2->SR = 0;
    TIM2->CR1 |= TIM_CR1_CEN;
}

// ================================================================
// CONFIGURACIÓN TIM3 (MULTIPLEXACIÓN)
// ================================================================
void TIM3_Multiplex_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    TIM3->PSC = TIM3_PSC_VAL - 1; // 16 MHz / 16 = 1 MHz (1 us por tick)
    TIM3->ARR = 2000 - 1;         // 2000 us = 2 ms por interrupción

    TIM3->DIER |= TIM_DIER_UIE;
    TIM3->SR = 0;
    NVIC_EnableIRQ(29);
    TIM3->CR1 |= TIM_CR1_CEN;
}

// ================================================================
// MAIN
// ================================================================
int main(void)
{
    GPIO_Init();
    TIM2_IC_Init();
    TIM3_Multiplex_Init();

    // Mostrar 0 al inicio
    display_freq = 0;
    Update_Display_Digits(display_freq);

    while (1)
    {
        // Actualizar la pantalla SOLO cuando ha pasado 1 segundo
        if (one_second_flag)
        {
            one_second_flag = 0; // Resetear la bandera
            Update_Display_Digits(display_freq);
        }
    }
}

// ================================================================
// INTERRUPCIÓN TIM2 (MEDICIÓN)
// ================================================================
void TIM2_IRQHandler(void)
{
    if (TIM2->SR & (1UL << 1))
    {
        uint32_t current_tick = TIM2->CCR1;
        uint32_t period_ticks = current_tick - last_capture_tick;
        last_capture_tick = current_tick;

        // Filtro de ruido abierto: 150 ticks a 16MHz permite medir hasta 106 kHz
        if (period_ticks > 150) 
        {
            uint32_t calc_freq = TIMER_CLOCK_HZ / period_ticks;
            
            // Acumular solo si está en el rango válido
            if (calc_freq >= MIN_FREQUENCY && calc_freq <= MAX_FREQUENCY) {
                freq_sum += calc_freq; // Utilizando sumador de 64 bits para no desbordar
                freq_count++;
            }
        }
        TIM2->SR &= ~(1UL << 1);
    }
}

// ================================================================
// INTERRUPCIÓN TIM3 (MULTIPLEXACIÓN Y TEMPORIZACIÓN 1s)
// ================================================================
void TIM3_IRQHandler(void)
{
    if (TIM3->SR & (1UL << 0))
    {
        // PASO 1: Apagar TODOS los dígitos
        GPIOB->BSRR = COM_OFF(0) | COM_OFF(1) | COM_OFF(2) | COM_OFF(3) | COM_OFF(4);

        // PASO 2: Obtener dígito actual y aplicar máscara
        uint8_t val = digits_to_display[current_digit];
        
        if (val > 9) val = 0; 
        
        GPIOC->BSRR = segment_mask_complete[val];
        
        // Segmento G (PA9)
        if (segment_map[val] & 0x40) {
            GPIOA->BSRR = SEG_ON(9);
        } else {
            GPIOA->BSRR = SEG_OFF(9);
        }

        // PASO 3: Encender el dígito actual
        GPIOB->BSRR = COM_ON(current_digit);

        // PASO 4: Siguiente dígito
        current_digit++;
        if (current_digit >= DISPLAY_DIGITS) current_digit = 0;

        // Generar evento cada 1 segundo (500 interrupciones * 2ms = 1s)
        static uint16_t irq_count = 0;
        irq_count++;
        if (irq_count >= 500)
        {
            irq_count = 0;
            
            // Timeout: si el último flanco fue hace mucho tiempo, no hay señal.
            uint32_t current_time = TIM2->CNT;
            if ((current_time - last_capture_tick) > 16000000UL) {
                display_freq = 0;
            } else if (freq_count > 0) {
                // Casteo seguro después de promediar
                display_freq = (uint32_t)(freq_sum / freq_count);
            } 
            
            // Reiniciar acumuladores
            freq_sum = 0;
            freq_count = 0;
            
            // Indicar al main que actualice la pantalla
            one_second_flag = 1;
        }

        TIM3->SR &= ~(1UL << 0);
    }
}

// ================================================================
// ACTUALIZAR DÍGITOS (MAPEO FÍSICO CORREGIDO)
// ================================================================
void Update_Display_Digits(uint32_t freq)
{
    if (freq > 99999) freq = 99999;
    
    // ORDEN FÍSICO CORREGIDO:
    // PB0 controla el dígito más significativo (10k)
    // PB4 controla el dígito menos significativo (unidades - el display individual)
    digits_to_display[0] = (freq / 10000) % 10; // D1: Decenas de mil
    digits_to_display[1] = (freq / 1000) % 10;  // D2: Unidades de mil
    digits_to_display[2] = (freq / 100) % 10;   // D3: Centenas
    digits_to_display[3] = (freq / 10) % 10;    // D4: Decenas
    digits_to_display[4] = freq % 10;           // D5: Unidades (Display individual)
}