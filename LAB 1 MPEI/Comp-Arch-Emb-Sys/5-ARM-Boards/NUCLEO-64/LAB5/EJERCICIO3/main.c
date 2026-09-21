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

#ifndef TIM1
#define TIM1 ((TIM_TypeDef *) 0x40010000)
#endif

// Macros de bits faltantes
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

// Definición completa de la estructura del Timer 1 (Avanzado) con BDTR incluido
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

// Lógica de hardware
#define SEGMENTS_ACTIVE_LOW  1 
#define COMMONS_ACTIVE_LOW   0 

// ================================================================
// DICCIONARIO ALFANUMÉRICO (7 SEGMENTOS)
// ================================================================
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

// Mapa hexadecimal para algunas letras y símbolos
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
volatile uint8_t current_song = 0; // 0 = Melodía 1, 1 = Melodía 2

// Variables para el scroll de texto
volatile uint8_t display_buffer[5] = {CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK};
const uint8_t text_play[] = {CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_P, CHAR_L, CHAR_A, CHAR_Y, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK};
const uint8_t text_paus[] = {CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_P, CHAR_A, CHAR_U, CHAR_S, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK};
const uint8_t text_stop[] = {CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_S, CHAR_T, CHAR_O, CHAR_P, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK};

volatile uint8_t scroll_index = 0;
volatile uint16_t scroll_timer = 0;

// Variables musicales
typedef struct {
    uint32_t frequency; // En Hz (0 = silencio)
    uint32_t duration;  // En milisegundos
} Note;

// Melodía 1 (Escala simple de prueba)
const Note song_1[] = {
    {261, 500}, {293, 500}, {329, 500}, {349, 500}, {392, 500}, {0, 0} // 0,0 indica fin de canción
};

// Melodía 2 (Arpegio rápido)
const Note song_2[] = {
    {440, 250}, {554, 250}, {659, 250}, {880, 500}, {0, 0}
};

const Note* playlist[] = {song_1, song_2};

volatile uint16_t note_index = 0;
volatile uint16_t note_timer = 0;

// *** FIX 3: debounce ***
// Contador de milisegundos (incrementado en SysTick_Handler) y última
// marca de tiempo en que cada botón fue realmente procesado. Sin esto,
// el rebote mecánico de un solo "click" genera decenas de flancos en
// pocos milisegundos, cada uno reentrando al ISR y saturando la CPU,
// lo que deja el multiplexado de displays (TIM3) congelado en el
// dígito que estaba activo en ese instante.
#define DEBOUNCE_MS 200
volatile uint32_t ms_ticks = 0;
volatile uint32_t last_press_ms[4] = {0, 0, 0, 0}; // 0=PC10,1=PC11,2=PC12,3=PC13

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
void Update_Scroll(void);
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

// *** FIX 4: diagnóstico de HardFault ***
// Si el sistema se "congela" de nuevo, esto lo distingue de otros
// problemas: enciende el LED de usuario de la Nucleo-64 (LD2, PA5) y
// se queda ahí fijo. Si al congelarse ves el LED encendido, confirmas
// que es un HardFault real (acceso inválido a memoria/periférico) y
// no solo el reproductor colgado esperando algo.
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
    Default_Handler,     // NMI
    HardFault_Handler,   // HardFault (FIX 4: antes era Default_Handler)
    Default_Handler, Default_Handler, 
    Default_Handler, Default_Handler, 0, 0, 0, 0,                       
    Default_Handler, Default_Handler, 0, SysTick_Handler, Default_Handler, 
    [16 + 29] = TIM3_IRQHandler,
    [16 + 40] = EXTI15_10_IRQHandler // Vector de EXTI 10 a 15 (IRQ 40)
};

// ================================================================
// INICIALIZACIÓN DE HARDWARE
// ================================================================
void GPIO_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN;

    // PA8 como Función Alternativa (PWM TIM1_CH1 para el PN100)
    GPIOA->MODER &= ~(3UL << 16);
    GPIOA->MODER |= (2UL << 16);
    GPIOA->AFR[1] &= ~(0xFUL << 0);
    GPIOA->AFR[1] |= (1UL << 0); // AF1 para TIM1

    // PC10, PC11, PC12, PC13 como entradas con Pull-Up (Botones)
    GPIOC->MODER &= ~((3UL << 20) | (3UL << 22) | (3UL << 24) | (3UL << 26));
    GPIOC->PUPDR &= ~((3UL << 20) | (3UL << 22) | (3UL << 24) | (3UL << 26));
    GPIOC->PUPDR |=  ((1UL << 20) | (1UL << 22) | (1UL << 24) | (1UL << 26));

    // PC0, PC3, PC4, PC5, PC7, PC8 (Segmentos A-F)
    uint32_t mask_c = (3UL << 0) | (3UL << 6) | (3UL << 8) | (3UL << 10) | (3UL << 14) | (3UL << 16);
    GPIOC->MODER &= ~mask_c;
    GPIOC->MODER |= (1UL << 0) | (1UL << 6) | (1UL << 8) | (1UL << 10) | (1UL << 14) | (1UL << 16);

    // PA9 (Segmento G)
    GPIOA->MODER &= ~(3UL << 18);
    GPIOA->MODER |= (1UL << 18);

    // PB0-PB4 (Transistores de Multiplexado)
    GPIOB->MODER &= ~0x000003FF;
    GPIOB->MODER |= 0x00000155;

    // Apagar todo por seguridad
    GPIOB->BSRR = COM_OFF(0) | COM_OFF(1) | COM_OFF(2) | COM_OFF(3) | COM_OFF(4);
}

void EXTI_Buttons_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; // Activa ruteo de EXTI

    // *** FIX 1 ***
    // Tras habilitar el reloj de SYSCFG hay que esperar a que se propague
    // antes de escribir sus registros (ver guía, pregunta 5f). Sin esta
    // lectura "dummy", SYSCFG->EXTICR puede quedar sin escribirse y
    // EXTI10-13 se queda enrutado por defecto a GPIOA (no hay botón ahí),
    // por lo que los botones en PC10-PC13 nunca generan la interrupción.
    volatile uint32_t dummy;
    dummy = (RCC->APB2ENR);
    dummy = (RCC->APB2ENR);
    (void)dummy;

    // Conectar PC10-PC13 a EXTI10-EXTI13
    SYSCFG->EXTICR[2] &= ~(0xFF00); 
    SYSCFG->EXTICR[2] |=  (0x2200); // PC10, PC11
    SYSCFG->EXTICR[3] &= ~(0x00FF); 
    SYSCFG->EXTICR[3] |=  (0x0022); // PC12, PC13

    // Interrupción por flanco de bajada (al presionar)
    EXTI->FTSR |= (1UL << 10) | (1UL << 11) | (1UL << 12) | (1UL << 13);
    EXTI->RTSR &= ~((1UL << 10) | (1UL << 11) | (1UL << 12) | (1UL << 13));

    EXTI->IMR |= (1UL << 10) | (1UL << 11) | (1UL << 12) | (1UL << 13); // Desenmascarar
    NVIC_EnableIRQ(40); // EXTI15_10_IRQn es la posición 40
}

void TIM1_PWM_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN; // Timer 1 está en el bus APB2 (16MHz)

    TIM1->PSC = 16 - 1; // Reloj del timer = 1 MHz
    TIM1->ARR = 0;      // Frecuencia en 0 (Silencio por defecto)
    
    // Configurar Modo PWM 1 en el Canal 1
    TIM1->CCMR1 &= ~TIM_CCMR1_CC1S;   // Canal como salida
    TIM1->CCMR1 |= (6UL << 4);        // OC1M = 110 (Modo PWM 1)
    TIM1->CCMR1 |= TIM_CCMR1_OC1PE;   // Preload enable
    
    TIM1->CCER |= TIM_CCER_CC1E;      // Habilitar salida en CH1
    TIM1->BDTR |= TIM_BDTR_MOE;       // Main Output Enable (CRÍTICO EN TIM1)
    
    TIM1->CCR1 = 0; // Ciclo de trabajo al 0%
    TIM1->CR1 |= TIM_CR1_CEN;
}

void TIM3_Multiplex_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    TIM3->PSC = 16 - 1;      // 1us por tick
    TIM3->ARR = 2000 - 1;    // 2ms
    TIM3->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(29);
    TIM3->CR1 |= TIM_CR1_CEN;
}

void SysTick_Init(void)
{
    // SysTick interrumpe cada 1ms para llevar el tiempo exacto de las notas
    SysTick->LOAD = 16000 - 1; 
    SysTick->VAL = 0;
    SysTick->CTRL = (1 << 2) | (1 << 1) | (1 << 0); // Reloj del procesador, interrupción habilitada, activar
}

// ================================================================
// CONTROL DE AUDIO Y ESTADOS
// ================================================================
void Play_Tone(uint32_t freq)
{
    if (freq == 0) {
        TIM1->CCR1 = 0; // 0% duty cycle (silencio)
    } else {
        // Cálculo matemático del PWM: ARR = (1,000,000 / freq) - 1
        uint32_t arr_val = (1000000UL / freq) - 1;
        TIM1->ARR = arr_val;
        TIM1->CCR1 = arr_val / 2; // 50% de ciclo de trabajo
    }
}

// *** FIX 2 ***
// Arranca la reproducción desde la primera nota de la canción actual.
// Antes, al pasar a STATE_PLAYING no se llamaba Play_Tone ni se
// inicializaba note_timer: como note_timer arrancaba en 0, el primer
// tick de SysTick_Handler incrementaba note_index ANTES de sonar nada,
// así que la primera nota de la canción siempre se saltaba.
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
    if (EXTI->PR & (1UL << 10)) // PC10: Play / Pause
    {
        if ((ms_ticks - last_press_ms[0]) >= DEBOUNCE_MS)
        {
            last_press_ms[0] = ms_ticks;
            if (current_state == STATE_PLAYING) {
                current_state = STATE_PAUSED;
                Play_Tone(0); // Silencia al pausar
            } else if (current_state == STATE_STOPPED) {
                // Empieza la canción desde el principio
                Start_Playback();
                current_state = STATE_PLAYING;
            } else { // STATE_PAUSED -> reanuda la nota en la que iba
                Play_Tone(playlist[current_song][note_index].frequency);
                current_state = STATE_PLAYING;
            }
            scroll_index = 0; // Reinicia el scroll
        }
        EXTI->PR = (1UL << 10); // Limpiar SIEMPRE, sea rebote o no
    }
    
    if (EXTI->PR & (1UL << 11)) // PC11: Stop
    {
        if ((ms_ticks - last_press_ms[1]) >= DEBOUNCE_MS)
        {
            last_press_ms[1] = ms_ticks;
            current_state = STATE_STOPPED;
            Play_Tone(0);
            note_index = 0; // Reinicia la canción
            scroll_index = 0;
        }
        EXTI->PR = (1UL << 11); 
    }
    
    if (EXTI->PR & (1UL << 12)) // PC12: Siguiente Canción
    {
        if ((ms_ticks - last_press_ms[2]) >= DEBOUNCE_MS)
        {
            last_press_ms[2] = ms_ticks;
            current_song = (current_song + 1) % 2; // Alterna entre 0 y 1
            note_index = 0;
            scroll_index = 0;
            if (current_state == STATE_PLAYING) {
                note_timer = playlist[current_song][0].duration;
                Play_Tone(playlist[current_song][0].frequency);
            }
        }
        EXTI->PR = (1UL << 12); 
    }

    if (EXTI->PR & (1UL << 13)) // PC13: Anterior
    {
        if ((ms_ticks - last_press_ms[3]) >= DEBOUNCE_MS)
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

// Interrupción de tiempo (1 milisegundo exacto)
void SysTick_Handler(void)
{
    ms_ticks++; // Base de tiempo para el debounce de los botones

    // 1. Manejo del reproductor musical
    if (current_state == STATE_PLAYING) 
    {
        if (note_timer > 0) {
            note_timer--; // Descuenta 1ms de la duración de la nota actual
        } else {
            // Se acabó el tiempo de la nota actual, pasamos a la siguiente
            note_index++;
            uint32_t next_freq = playlist[current_song][note_index].frequency;
            uint32_t next_duration = playlist[current_song][note_index].duration;

            if (next_duration == 0) { // Encontramos el final (0,0)
                current_state = STATE_STOPPED;
                note_index = 0;
                Play_Tone(0);
            } else {
                Play_Tone(next_freq);
                note_timer = next_duration;
            }
        }
    }

    // 2. Manejo del texto deslizante (Scroll de 500ms)
    scroll_timer++;
    if (scroll_timer >= 500) 
    {
        scroll_timer = 0;
        const uint8_t* active_text;
        uint8_t text_length = 14; 

        if (current_state == STATE_PLAYING) active_text = text_play;
        else if (current_state == STATE_PAUSED) active_text = text_paus;
        else active_text = text_stop;

        // Carga 5 letras consecutivas en los 5 displays
        for (int i = 0; i < 5; i++) {
            display_buffer[i] = active_text[(scroll_index + i) % text_length];
        }

        scroll_index++;
        if (scroll_index >= text_length) scroll_index = 0;
    }
}

void TIM3_IRQHandler(void)
{
    if (TIM3->SR & TIM_SR_UIF)
    {
        static uint8_t current_digit = 0;

        // Apagar todos los displays (Ghosting)
        GPIOB->BSRR = COM_OFF(0) | COM_OFF(1) | COM_OFF(2) | COM_OFF(3) | COM_OFF(4);

        uint8_t pattern = display_buffer[current_digit];

        // Apagar segmentos por defecto
        GPIOC->BSRR = SEG_OFF(0) | SEG_OFF(3) | SEG_OFF(4) | SEG_OFF(5) | SEG_OFF(7) | SEG_OFF(8);
        GPIOA->BSRR = SEG_OFF(9);

        // Encender dinámicamente los segmentos que dicte la letra
        if (pattern & 0x01) GPIOC->BSRR = SEG_ON(0); // A
        if (pattern & 0x02) GPIOC->BSRR = SEG_ON(7); // B
        if (pattern & 0x04) GPIOC->BSRR = SEG_ON(8); // C
        if (pattern & 0x08) GPIOC->BSRR = SEG_ON(3); // D
        if (pattern & 0x10) GPIOC->BSRR = SEG_ON(4); // E
        if (pattern & 0x20) GPIOC->BSRR = SEG_ON(5); // F
        if (pattern & 0x40) GPIOA->BSRR = SEG_ON(9); // G

        // Encender dígito
        GPIOB->BSRR = COM_ON(current_digit);

        current_digit++;
        if (current_digit >= DISPLAY_DIGITS) current_digit = 0;

        TIM3->SR &= ~TIM_SR_UIF;
    }
}

int main(void)
{
    GPIO_Init();
    TIM1_PWM_Init();
    TIM3_Multiplex_Init();
    EXTI_Buttons_Init();
    SysTick_Init();

    // Inicia el sistema
    current_state = STATE_STOPPED;
    Play_Tone(0);

    while (1) {
        // En un sistema por interrupciones, el main no hace nada. 
        // Se queda durmiendo ahorrando energía.
    }
}