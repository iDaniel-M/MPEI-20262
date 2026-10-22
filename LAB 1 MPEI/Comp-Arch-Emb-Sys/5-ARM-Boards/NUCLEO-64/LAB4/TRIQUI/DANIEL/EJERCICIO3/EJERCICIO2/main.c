#include "stm32f446xx.h" // libreria con los registros del microcontrolador stm32f446re
#include <stdint.h>      // tipos de datos estandar como uint32_t o uint8_t

#define TIMER_CLOCK_HZ       16000000UL // velocidad base del reloj del sistema a 16 mhz
#define TIM3_PSC_VAL         16         // presescalador para que el timer 3 cuente en microsegundos

#define MIN_FREQUENCY        1000UL     // limite minimo de lectura que permite el sistema
#define MAX_FREQUENCY        55000UL    // limite maximo de lectura permitido
#define DISPLAY_DIGITS       5U         // cantidad de pantallas de 7 segmentos en uso

#define SEGMENTS_ACTIVE_LOW  1 // los leds de los segmentos encienden con cero voltios
#define COMMONS_ACTIVE_LOW   0 // los transistores npn encienden con tres punto tres voltios

void GPIO_Init(void);                      // inicializa las entradas y salidas del micro
void TIM2_IC_Init(void);                   // inicializa el timer 2 en modo captura de entrada
void TIM3_Multiplex_Init(void);            // inicializa el timer 3 para el barrido de pantallas
void Update_Display_Digits(uint32_t freq); // descompone la frecuencia en digitos individuales
void TIM2_IRQHandler(void);                // interrupcion del timer 2 al capturar un flanco
void TIM3_IRQHandler(void);                // interrupcion del timer 3 cada dos milisegundos
int main(void);                            // funcion principal de ejecucion

extern uint32_t _estack; // direccion inicial del stack pointer que viene del linker script

void Reset_Handler(void) { main(); while (1); } // primer codigo que arranca al encender
void Default_Handler(void) { while (1); }       // trampa de seguridad para interrupciones no usadas

__attribute__((section(".isr_vector"))) // guarda este arreglo directo en la flash
void (* const g_pfnVectors[])(void) =
{
    (void (*)(void))(&_estack), // posicion inicial de la pila
    Reset_Handler,              // posicion del vector de reset
    Default_Handler, Default_Handler, Default_Handler, Default_Handler, // excepciones del nucleo arm
    Default_Handler, Default_Handler, 0, 0, 0, 0,                       // espacios reservados
    Default_Handler, Default_Handler, 0, Default_Handler, Default_Handler, // interrupciones del sistema
    [16 + 28] = TIM2_IRQHandler, // posicion de la irq 28 para la interrupcion del timer 2
    [16 + 29] = TIM3_IRQHandler, // posicion de la irq 29 para la interrupcion del timer 3
};

volatile uint8_t current_digit = 0; // indica que pantalla se esta encendiendo actualmente
volatile uint8_t digits_to_display[DISPLAY_DIGITS] = {0, 0, 0, 0, 0}; // arreglo con los valores a dibujar

volatile uint64_t freq_sum = 0;       // acumulador de frecuencias de 64 bits para evitar desbordamiento
volatile uint32_t freq_count = 0;     // contador de lecturas capturadas en un segundo
volatile uint32_t display_freq = 0;   // frecuencia promediada final lista para mostrar
volatile uint8_t one_second_flag = 0; // bandera que avisa al main cuando paso un segundo completo

volatile uint32_t last_capture_tick = 0; // guarda la hora en ticks del ultimo flanco recibido

#if SEGMENTS_ACTIVE_LOW
    #define SEG_OFF(pin) (1UL << (pin))        // apaga el segmento enviando tres punto tres voltios
    #define SEG_ON(pin)  (1UL << ((pin) + 16)) // enciende el segmento mandando a tierra mediante bsrr
#else
    #define SEG_OFF(pin) (1UL << ((pin) + 16))
    #define SEG_ON(pin)  (1UL << (pin))
#endif

#if COMMONS_ACTIVE_LOW
    #define COM_OFF(pin) (1UL << (pin))
    #define COM_ON(pin)  (1UL << ((pin) + 16))
#else
    #define COM_OFF(pin) (1UL << ((pin) + 16)) // apaga el transistor npn mandando cero voltios a la base
    #define COM_ON(pin)  (1UL << (pin))        // enciende el transistor npn mandando tres punto tres voltios
#endif

const uint8_t segment_map[10] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F // patron de segmentos para los numeros cero al nueve
};

const uint32_t segment_mask_complete[10] = {
    // mascaras de 32 bits creadas para encender y apagar segmentos en un solo ciclo usando bsrr
    SEG_ON(0) | SEG_ON(7) | SEG_ON(8) | SEG_ON(3) | SEG_ON(4) | SEG_ON(5), // numero 0
    SEG_OFF(0) | SEG_ON(7) | SEG_ON(8) | SEG_OFF(3) | SEG_OFF(4) | SEG_OFF(5), // numero 1
    SEG_ON(0) | SEG_ON(7) | SEG_OFF(8) | SEG_ON(3) | SEG_ON(4) | SEG_OFF(5), // numero 2
    SEG_ON(0) | SEG_ON(7) | SEG_ON(8) | SEG_ON(3) | SEG_OFF(4) | SEG_OFF(5), // numero 3
    SEG_OFF(0) | SEG_ON(7) | SEG_ON(8) | SEG_OFF(3) | SEG_OFF(4) | SEG_ON(5), // numero 4
    SEG_ON(0) | SEG_OFF(7) | SEG_ON(8) | SEG_ON(3) | SEG_OFF(4) | SEG_ON(5), // numero 5
    SEG_ON(0) | SEG_OFF(7) | SEG_ON(8) | SEG_ON(3) | SEG_ON(4) | SEG_ON(5), // numero 6
    SEG_ON(0) | SEG_ON(7) | SEG_ON(8) | SEG_OFF(3) | SEG_OFF(4) | SEG_OFF(5), // numero 7
    SEG_ON(0) | SEG_ON(7) | SEG_ON(8) | SEG_ON(3) | SEG_ON(4) | SEG_ON(5), // numero 8
    SEG_ON(0) | SEG_ON(7) | SEG_ON(8) | SEG_ON(3) | SEG_OFF(4) | SEG_ON(5)  // numero 9
};

void GPIO_Init(void)
{
    // activa los relojes para los puertos a, b y c
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN;

    // configura pa0 en modo funcion alternativa para recibir la entrada del tim2_ch1
    GPIOA->MODER &= ~(3UL << 0); // limpia los bits del pin pa0
    GPIOA->MODER |= (2UL << 0);  // selecciona modo funcion alternativa
    GPIOA->AFR[0] &= ~(0xFUL << 0); // limpia los bits de funcion alternativa baja
    GPIOA->AFR[0] |= (1UL << 0); // asigna la funcion alternativa af1 correspondiente al timer 2
    GPIOA->PUPDR &= ~(3UL << 0); // limpia configuracion de pullup o pulldown
    GPIOA->PUPDR |= (2UL << 0);  // habilita resistencia pulldown interna en pa0

    // configura pc0, pc3, pc4, pc5, pc7 y pc8 como salidas para los segmentos del display
    uint32_t mask_c = (3UL << 0) | (3UL << 6) | (3UL << 8) | (3UL << 10) | (3UL << 14) | (3UL << 16);
    GPIOC->MODER &= ~mask_c; // limpia los bits de los pines seleccionados
    GPIOC->MODER |= (1UL << 0) | (1UL << 6) | (1UL << 8) | (1UL << 10) | (1UL << 14) | (1UL << 16); // modo salida

    // configura pa9 como salida para el segmento g
    GPIOA->MODER &= ~(3UL << 18); // limpia el pin pa9
    GPIOA->MODER |= (1UL << 18);  // modo salida

    // configura pb0 a pb4 como salidas para controlar los transistores de cada digito
    GPIOB->MODER &= ~0x000003FF; // limpia los primeros cinco pines del puerto b
    GPIOB->MODER |= 0x00000155;  // los establece en modo salida digital

    // apaga todos los transistores y segmentos al iniciar para evitar destellos
    GPIOB->BSRR = COM_OFF(0) | COM_OFF(1) | COM_OFF(2) | COM_OFF(3) | COM_OFF(4);
    GPIOC->BSRR = SEG_OFF(0) | SEG_OFF(3) | SEG_OFF(4) | SEG_OFF(5) | SEG_OFF(7) | SEG_OFF(8);
    GPIOA->BSRR = SEG_OFF(9);
}

void TIM2_IC_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // energiza el modulo del timer 2 en el bus apb1
    TIM2->PSC = 0;          // presescalador en cero para contar directo a dieciseis megahertz
    TIM2->ARR = 0xFFFFFFFF; // valor maximo de conteo antes de dar la vuelta completa

    TIM2->CCMR1 &= ~(3UL << 0); // limpia seleccion de canal
    TIM2->CCMR1 |= (1UL << 0);  // configura el canal 1 como entrada conectada al pin ti1

    TIM2->CCMR1 &= ~(0xFUL << 4); // limpia los bits del filtro
    TIM2->CCMR1 |= (4UL << 4);    // aplica un filtro de ocho ciclos de reloj para limpiar la señal

    TIM2->CCER &= ~((1UL << 1) | (1UL << 3)); // detecta flancos de subida
    TIM2->CCER |= (1UL << 0);  // habilita la captura en el canal 1

    TIM2->DIER |= TIM_DIER_CC1IE; // activa la interrupcion por captura de canal 1
    NVIC_EnableIRQ(28); // habilita el vector numero 28 en el controlador de interrupciones nvic
    
    TIM2->CNT = 0; // resetea el contador principal a cero
    TIM2->SR = 0;  // limpia cualquier bandera pendiente
    TIM2->CR1 |= TIM_CR1_CEN; // enciende el contador del timer 2
}

void TIM3_Multiplex_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; // habilita el reloj del timer 3

    TIM3->PSC = TIM3_PSC_VAL - 1; // presescalador para tener ticks de un microsegundo
    TIM3->ARR = 2000 - 1;         // interrumpe cada dos mil microsegundos o sea dos milisegundos

    TIM3->DIER |= TIM_DIER_UIE; // activa interrupcion por desbordamiento
    TIM3->SR = 0; // limpia banderas
    NVIC_EnableIRQ(29); // habilita la interrupcion 29 del timer 3 en el nvic
    TIM3->CR1 |= TIM_CR1_CEN; // enciende el contador del timer 3
}

int main(void)
{
    GPIO_Init();           // inicializa las entradas y salidas
    TIM2_IC_Init();        // inicializa el timer de medicion
    TIM3_Multiplex_Init(); // inicializa el timer de multiplexado

    display_freq = 0; // frecuencia inicial en cero
    Update_Display_Digits(display_freq); // carga ceros en el arreglo de visualizacion

    while (1)
    {
        if (one_second_flag) // espera a que transcurra un segundo completo
        {
            one_second_flag = 0; // baja la bandera de segundo
            Update_Display_Digits(display_freq); // actualiza los digitos con la nueva frecuencia
        }
    }
}

void TIM2_IRQHandler(void)
{
    if (TIM2->SR & (1UL << 1)) // comprueba si la interrupcion fue por captura de flanco
    {
        uint32_t current_tick = TIM2->CCR1; // lee el momento exacto del flanco recibido
        uint32_t period_ticks = current_tick - last_capture_tick; // calcula los ticks transcurridos entre flancos
        last_capture_tick = current_tick; // actualiza el registro de la ultima captura

        if (period_ticks > 150) // ignora periodos muy cortos para filtrar ruido por encima de 106 kilohertz
        {
            uint32_t calc_freq = TIMER_CLOCK_HZ / period_ticks; // calcula la frecuencia instatanea
            
            if (calc_freq >= MIN_FREQUENCY && calc_freq <= MAX_FREQUENCY) { // verifica el rango valido
                freq_sum += calc_freq; // acumula la frecuencia calculada
                freq_count++;          // suma una muestra valida capturada
            }
        }
        TIM2->SR &= ~(1UL << 1); // limpia la bandera de interrupcion de captura
    }
}

void TIM3_IRQHandler(void)
{
    if (TIM3->SR & (1UL << 0)) // comprueba si la interrupcion es del timer 3
    {
        GPIOB->BSRR = COM_OFF(0) | COM_OFF(1) | COM_OFF(2) | COM_OFF(3) | COM_OFF(4); // apaga pantallas para evitar fantasmas

        uint8_t val = digits_to_display[current_digit]; // toma el valor correspondiente a esta pantalla
        
        if (val > 9) val = 0; // proteccion en caso de leer un indice fuera de rango
        
        GPIOC->BSRR = segment_mask_complete[val]; // enciende los segmentos necesarios en puerto c
        
        if (segment_map[val] & 0x40) {
            GPIOA->BSRR = SEG_ON(9); // enciende el segmento g en pa9 si aplica
        } else {
            GPIOA->BSRR = SEG_OFF(9); // apaga el segmento g
        }

        GPIOB->BSRR = COM_ON(current_digit); // enciende el transistor de la pantalla activa

        current_digit++; // avanza a la siguiente pantalla
        if (current_digit >= DISPLAY_DIGITS) current_digit = 0; // si llego al final vuelve a la primera

        static uint16_t irq_count = 0; // contador interno para medir un segundo
        irq_count++;
        if (irq_count >= 500) // quinientas interrupciones de dos milisegundos equivalen a un segundo
        {
            irq_count = 0; // reinicia el contador de interrupciones
            
            uint32_t current_time = TIM2->CNT; // consulta el contador del timer 2
            if ((current_time - last_capture_tick) > 16000000UL) { // si pasa un segundo sin flancos
                display_freq = 0; // muestra cero en pantalla por falta de señal
            } else if (freq_count > 0) {
                display_freq = (uint32_t)(freq_sum / freq_count); // saca el promedio de las muestras capturadas
            } 
            
            freq_sum = 0;   // resetea la suma para el siguiente segundo
            freq_count = 0; // resetea el contador de muestras
            
            one_second_flag = 1; // levanta la bandera para actualizar la pantalla desde el main
        }

        TIM3->SR &= ~(1UL << 0); // limpia la bandera de interrupcion del timer 3
    }
}

void Update_Display_Digits(uint32_t freq)
{
    if (freq > 50000) freq = 50000; // limita el tope visual al rango pedido
    
    // extrae cada digito individualmente respetando el orden fisico asignado a tus pantallas
    digits_to_display[0] = (freq / 10000) % 10; // decenas de millar
    digits_to_display[1] = (freq / 1000) % 10;  // unidades de millar
    digits_to_display[2] = (freq / 100) % 10;   // centenas
    digits_to_display[3] = (freq / 10) % 10;    // decenas
    digits_to_display[4] = freq % 10;           // unidades
}