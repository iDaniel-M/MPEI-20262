#include "stm32f446xx.h"
#include <stdint.h>

// ================================================================
// DEFINICIONES FALTANTES EN LA CABECERA BARE-METAL
// ================================================================
#ifndef SYSCFG
typedef struct {
    volatile uint32_t MEMRMP;
    volatile uint32_t PMC;
    volatile uint32_t EXTICR[4];
    volatile uint32_t CMPCR;
} SYSCFG_TypeDef;
#define SYSCFG ((SYSCFG_TypeDef *) 0x40013800)
#endif

#ifndef EXTI
typedef struct {
    volatile uint32_t IMR;
    volatile uint32_t EMR;
    volatile uint32_t RTSR;
    volatile uint32_t FTSR;
    volatile uint32_t SWIER;
    volatile uint32_t PR;
} EXTI_TypeDef;
#define EXTI ((EXTI_TypeDef *) 0x40013C00)
#endif

#ifndef RCC_APB2ENR_SYSCFGEN
#define RCC_APB2ENR_SYSCFGEN (1UL << 14)
#endif

#ifndef RCC_APB2ENR_TIM1EN
#define RCC_APB2ENR_TIM1EN (1UL << 0)
#endif

#ifndef TIM_CCMR1_CC1S
#define TIM_CCMR1_CC1S (3UL << 0)
#endif

#ifndef TIM_CCMR1_OC1PE
#define TIM_CCMR1_OC1PE (1UL << 3)
#endif

#ifndef TIM_BDTR_MOE
#define TIM_BDTR_MOE (1UL << 15)
#endif

// Estructura completa del Timer 1 (Avanzado)
typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SMCR;
    volatile uint32_t DIER;
    volatile uint32_t SR;
    volatile uint32_t EGR;
    volatile uint32_t CCMR1;
    volatile uint32_t CCMR2;
    volatile uint32_t CCER;
    volatile uint32_t CNT;
    volatile uint32_t PSC;
    volatile uint32_t ARR;
    volatile uint32_t RCR;
    volatile uint32_t CCR1;
    volatile uint32_t CCR2;
    volatile uint32_t CCR3;
    volatile uint32_t CCR4;
    volatile uint32_t BDTR;
} TIM1_TypeDef;

#undef TIM1
#define TIM1 ((TIM1_TypeDef *) 0x40010000)

// ================================================================
// CONFIGURACIÓN DEL SISTEMA
// ================================================================
#define TIMER_CLOCK_HZ       16000000UL 
#define DISPLAY_DIGITS       5U

#define SEGMENTS_ACTIVE_LOW  1 
#define COMMONS_ACTIVE_LOW   0 

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

#define CHAR_P 0x73
#define CHAR_L 0x38
#define CHAR_A 0x77
#define CHAR_Y 0x6E
#define CHAR_U 0x3E
#define CHAR_S 0x6D
#define CHAR_T 0x78
#define CHAR_O 0x3F
#define CHAR_BLANK 0x00

// ================================================================
// MÁQUINA DE ESTADOS Y MÚSICA
// ================================================================
typedef enum {
    STATE_STOPPED,
    STATE_PLAYING,
    STATE_PAUSED
} PlayerState;

volatile PlayerState current_state = STATE_STOPPED;
volatile uint8_t current_song = 0; 

volatile uint8_t display_buffer[5] = {CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK};

#define TEXT_LEN 9
const uint8_t text_play[TEXT_LEN] = {CHAR_P, CHAR_L, CHAR_A, CHAR_Y, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK};
const uint8_t text_paus[TEXT_LEN] = {CHAR_P, CHAR_A, CHAR_U, CHAR_S, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK};
const uint8_t text_stop[TEXT_LEN] = {CHAR_S, CHAR_T, CHAR_O, CHAR_P, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK};

volatile uint8_t scroll_index = 0;
volatile uint16_t scroll_timer = 0;

typedef struct {
    uint32_t frequency; 
    uint32_t duration;  
} Note;

const Note song_1[] = {
    {261, 500}, {293, 500}, {329, 500}, {349, 500}, {392, 500}, {0, 0} 
};

const Note song_2[] = {
    {440, 250}, {554, 250}, {659, 250}, {880, 500}, {0, 0}
};

const Note* playlist[] = {song_1, song_2};

volatile uint16_t note_index = 0;
volatile uint16_t note_timer = 0;

#define DEBOUNCE_MS 200
volatile uint32_t ms_ticks = 0;
// Inicializamos con un valor negativo grande para permitir la primera pulsación inmediata sin bloqueos al arrancar
volatile int32_t last_press_ms[4] = {-300, -300, -300, -300}; 

// ================================================================
// PROTOTIPOS
// ================================================================
void GPIO_Init(void);
void EXTI_Buttons_Init(void);
void TIM1_PWM_Init(void);
void TIM3_Multiplex_Init(void);
void SysTick_Init(void);
void Play_Tone(uint32_t freq);
void Start_Playback(void);
int main(void);

// ================================================================
// TABLA DE VECTORES
// ================================================================
extern uint32_t _estack;
void EXTI15_10_IRQHandler(void);
void TIM3_IRQHandler(void);
void SysTick_Handler(void);
void HardFault_Handler(void);

void Reset_Handler(void) { main(); while (1); }
void Default_Handler(void) { while (1); }

void HardFault_Handler(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    GPIOA->MODER &= ~(3UL << (5 * 2));
    GPIOA->MODER |=  (1UL << (5 * 2));
    GPIOA->BSRR = (1UL << 5); 
    while (1);
}

__attribute__((section(".isr_vector")))
void (* const g_pfnVectors[])(void) =
{
    (void (*)(void))(&_estack),
    Reset_Handler,
    Default_Handler,     
    HardFault_Handler,   
    Default_Handler, Default_Handler, 
    Default_Handler, Default_Handler, 0, 0, 0, 0,                       
    Default_Handler, Default_Handler, 0, SysTick_Handler, Default_Handler, 
    [16 + 29] = TIM3_IRQHandler,
    [16 + 40] = EXTI15_10_IRQHandler 
};

// ================================================================
// INICIALIZACIÓN DE HARDWARE
// ================================================================
void GPIO_Init(void)
{
    // Habilita reloj para puertos GPIO A, B y C
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN;

    // Configura PA8 como función alternativa PWM (TIM1_CH1)
    GPIOA->MODER &= ~(3UL << 16);
    GPIOA->MODER |= (2UL << 16);
    GPIOA->AFR[1] &= ~(0xFUL << 0);
    GPIOA->AFR[1] |= (1UL << 0); 

    // Configura PC10, PC11, PC12, PC13 como entradas digitales con resistencia Pull-Up interna
    GPIOC->MODER &= ~((3UL << 20) | (3UL << 22) | (3UL << 24) | (3UL << 26));
    GPIOC->PUPDR &= ~((3UL << 20) | (3UL << 22) | (3UL << 24) | (3UL << 26));
    GPIOC->PUPDR |=  ((1UL << 20) | (1UL << 22) | (1UL << 24) | (1UL << 26));

    // Configuración de pines de segmentos del display en Puerto C
    uint32_t mask_c = (3UL << 0) | (3UL << 6) | (3UL << 8) | (3UL << 10) | (3UL << 14) | (3UL << 16);
    GPIOC->MODER &= ~mask_c;
    GPIOC->MODER |= (1UL << 0) | (1UL << 6) | (1UL << 8) | (1UL << 10) | (1UL << 14) | (1UL << 16);

    // Configura PA9 como salida para el segmento G
    GPIOA->MODER &= ~(3UL << 18);
    GPIOA->MODER |= (1UL << 18);

    // Configura PB0-PB4 como salidas para el multiplexado de transistores
    GPIOB->MODER &= ~0x000003FF;
    GPIOB->MODER |= 0x00000155;
    GPIOB->BSRR = COM_OFF(0) | COM_OFF(1) | COM_OFF(2) | COM_OFF(3) | COM_OFF(4);
}

void EXTI_Buttons_Init(void)
{
    // Habilita el reloj del multiplexor de interrupciones externas (SYSCFG)
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; 

    // Retardo de propagación para estabilizar el bus SYSCFG
    volatile uint32_t dummy;
    dummy = (RCC->APB2ENR);
    dummy = (RCC->APB2ENR);
    (void)dummy;

    // Conecta las líneas EXTI 10 a 13 al Puerto C (valor 0x2)
    SYSCFG->EXTICR[2] &= ~(0xFF00); 
    SYSCFG->EXTICR[2] |=  (0x2200); 
    SYSCFG->EXTICR[3] &= ~(0x00FF); 
    SYSCFG->EXTICR[3] |=  (0x0022); 

    // Configura interrupción por flanco de bajada (al presionar el botón se conecta a tierra)
    EXTI->FTSR |= (1UL << 10) | (1UL << 11) | (1UL << 12) | (1UL << 13);
    EXTI->RTSR &= ~((1UL << 10) | (1UL << 11) | (1UL << 12) | (1UL << 13));

    // Desenmascara las interrupciones en el controlador EXTI
    EXTI->IMR |= (1UL << 10) | (1UL << 11) | (1UL << 12) | (1UL << 13); 
    NVIC_EnableIRQ(40); // Habilita la línea combinada EXTI15_10 en el NVIC
}

void TIM1_PWM_Init(void)
{
    // Habilita reloj para el Timer Avanzado 1 en APB2
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN; 

    // Prescaler para configurar resolución de 1 microsegundo por tick (16 MHz / 16)
    TIM1->PSC = 16 - 1; 
    
    // Auto-Reload inicial seguro para evitar bloqueo del timer
    TIM1->ARR = 1000;      
    
    // Configura modo PWM 1 en el canal 1 (PA8)
    TIM1->CCMR1 &= ~TIM_CCMR1_CC1S;   
    TIM1->CCMR1 |= (6UL << 4);        
    TIM1->CCMR1 |= TIM_CCMR1_OC1PE;   
    
    TIM1->CCER |= TIM_CCER_CC1E;      
    TIM1->BDTR |= TIM_BDTR_MOE;       // Obligatorio en timers avanzados para habilitar salidas físicas
    
    // Ciclo de trabajo inicial al 0% (silencio)
    TIM1->CCR1 = 0; 
    
    // Inicia el contador del timer
    TIM1->CR1 |= TIM_CR1_CEN;
}

void TIM3_Multiplex_Init(void)
{
    // Timer 3 para refrescar los displays de 7 segmentos cada 2 ms
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    TIM3->PSC = 16 - 1;      
    TIM3->ARR = 2000 - 1;    
    TIM3->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(29);
    TIM3->CR1 |= TIM_CR1_CEN;
}

void SysTick_Init(void)
{
    // SysTick configurado para interrumpir exactamente cada 1 milisegundo
    SysTick->LOAD = 16000 - 1; 
    SysTick->VAL = 0;
    SysTick->CTRL = (1 << 2) | (1 << 1) | (1 << 0); 
}

// ================================================================
// CONTROL DE AUDIO Y ESTADOS
// ================================================================
void Play_Tone(uint32_t freq)
{
    if (freq == 0) {
        // Establece ciclo de trabajo en 0% para generar silencio
        TIM1->CCR1 = 0; 
    } else {
        // Calcula matemáticamente el periodo del PWM para la frecuencia deseada
        uint32_t arr_val = (1000000UL / freq) - 1;
        TIM1->ARR = arr_val;
        TIM1->CCR1 = arr_val / 2; // Ciclo de trabajo al 50% para máxima amplitud sonora
    }
}

void Start_Playback(void)
{
    note_index = 0;
    note_timer = playlist[current_song][0].duration;
    Play_Tone(playlist[current_song][0].frequency);
}

// ================================================================
// MANEJO DE INTERRUPCIONES
// ================================================================
void EXTI15_10_IRQHandler(void)
{
    // Interrupción del pin PC10 (Botón Play / Pause)
    if (EXTI->PR & (1UL << 10)) 
    {
        if (((int32_t)ms_ticks - last_press_ms[0]) >= DEBOUNCE_MS)
        {
            last_press_ms[0] = ms_ticks;
            if (current_state == STATE_PLAYING) {
                current_state = STATE_PAUSED;
                Play_Tone(0); 
            } else if (current_state == STATE_STOPPED) {
                Start_Playback();
                current_state = STATE_PLAYING;
            } else { 
                Play_Tone(playlist[current_song][note_index].frequency);
                current_state = STATE_PLAYING;
            }
            scroll_index = 0; 
        }
        EXTI->PR = (1UL << 10); // Limpia la bandera de interrupción escribiendo un 1
    }
    
    // Interrupción del pin PC11 (Botón Stop)
    if (EXTI->PR & (1UL << 11)) 
    {
        if (((int32_t)ms_ticks - last_press_ms[1]) >= DEBOUNCE_MS)
        {
            last_press_ms[1] = ms_ticks;
            current_state = STATE_STOPPED;
            Play_Tone(0);
            note_index = 0; 
            scroll_index = 0;
        }
        EXTI->PR = (1UL << 11); 
    }
    
    // Interrupción del pin PC12 (Botón Siguiente Canción)
    if (EXTI->PR & (1UL << 12)) 
    {
        if (((int32_t)ms_ticks - last_press_ms[2]) >= DEBOUNCE_MS)
        {
            last_press_ms[2] = ms_ticks;
            current_song = (current_song + 1) % 2; 
            note_index = 0;
            scroll_index = 0;
            if (current_state == STATE_PLAYING) {
                note_timer = playlist[current_song][0].duration;
                Play_Tone(playlist[current_song][0].frequency);
            }
        }
        EXTI->PR = (1UL << 12); 
    }

    // Interrupción del pin PC13 (Botón Anterior Canción / Botón Azul de la Placa)
    if (EXTI->PR & (1UL << 13)) 
    {
        if (((int32_t)ms_ticks - last_press_ms[3]) >= DEBOUNCE_MS)
        {
            last_press_ms[3] = ms_ticks;
            current_song = (current_song == 0) ? 1 : 0; 
            note_index = 0;
            scroll_index = 0;
            if (current_state == STATE_PLAYING) {
                note_timer = playlist[current_song][0].duration;
                Play_Tone(playlist[current_song][0].frequency);
            }
        }
        EXTI->PR = (1UL << 13); 
    }
}

void SysTick_Handler(void)
{
    ms_ticks++; // Incrementa el contador de milisegundos global para el antirrebote

    // Lógica del reproductor musical nota por nota
    if (current_state == STATE_PLAYING) 
    {
        if (note_timer > 0) {
            note_timer--; 
        } else {
            note_index++;
            uint32_t next_freq = playlist[current_song][note_index].frequency;
            uint32_t next_duration = playlist[current_song][note_index].duration;

            if (next_duration == 0) { // Fin de la canción actual
                current_state = STATE_STOPPED;
                note_index = 0;
                Play_Tone(0);
            } else {
                Play_Tone(next_freq);
                note_timer = next_duration;
            }
        }
    }

    // Lógica de desplazamiento del texto en los displays (cada 500 ms)
    scroll_timer++;
    if (scroll_timer >= 500) 
    {
        scroll_timer = 0;
        const uint8_t* active_text;

        if (current_state == STATE_PLAYING) active_text = text_play;
        else if (current_state == STATE_PAUSED) active_text = text_paus;
        else active_text = text_stop;

        for (int i = 0; i < 5; i++) {
            display_buffer[i] = active_text[(scroll_index + i) % TEXT_LEN];
        }

        scroll_index++;
        if (scroll_index >= TEXT_LEN) scroll_index = 0;
    }
}

void TIM3_IRQHandler(void)
{
    // Interrupción de multiplexado de 7 segmentos (cada 2 ms)
    if (TIM3->SR & TIM_SR_UIF) 
    {
        static uint8_t current_digit = 0; 

        // Apaga todos los transistores para evitar efecto fantasma
        GPIOB->BSRR = COM_OFF(0) | COM_OFF(1) | COM_OFF(2) | COM_OFF(3) | COM_OFF(4);

        uint8_t pattern = display_buffer[current_digit];

        // Apaga los segmentos del display actual
        GPIOC->BSRR = SEG_OFF(0) | SEG_OFF(3) | SEG_OFF(4) | SEG_OFF(5) | SEG_OFF(7) | SEG_OFF(8);
        GPIOA->BSRR = SEG_OFF(9);

        // Enciende los segmentos según el patrón de la letra
        if (pattern & 0x01) GPIOC->BSRR = SEG_ON(0); 
        if (pattern & 0x02) GPIOC->BSRR = SEG_ON(7); 
        if (pattern & 0x04) GPIOC->BSRR = SEG_ON(8); 
        if (pattern & 0x08) GPIOC->BSRR = SEG_ON(3); 
        if (pattern & 0x10) GPIOC->BSRR = SEG_ON(4); 
        if (pattern & 0x20) GPIOC->BSRR = SEG_ON(5); 
        if (pattern & 0x40) GPIOA->BSRR = SEG_ON(9); 

        // Enciende el dígito correspondiente
        GPIOB->BSRR = COM_ON(current_digit);

        current_digit++;
        if (current_digit >= DISPLAY_DIGITS) current_digit = 0;

        TIM3->SR &= ~TIM_SR_UIF;
    }
}

int main(void)
{
    // Inicialización de periféricos
    GPIO_Init();
    TIM1_PWM_Init();
    TIM3_Multiplex_Init();
    EXTI_Buttons_Init();
    SysTick_Init();

    current_state = STATE_STOPPED;
    Play_Tone(0);

    while (1) {
        // Bucle vacío. Las interrupciones gestionan toda la lógica en segundo plano.
    }
}