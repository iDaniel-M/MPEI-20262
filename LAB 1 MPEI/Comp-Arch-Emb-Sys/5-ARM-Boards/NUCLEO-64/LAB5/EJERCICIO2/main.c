#include "stm32f446xx.h" // Importa el mapa de memoria y registros específicos del microcontrolador STM32F446RE
#include <stdint.h>      // Permite usar tipos de datos de tamaño fijo estandarizados como uint32_t, uint8_t, etc.

// ================================================================
// CONFIGURACIÓN DEL SISTEMA
// ================================================================

#define TIMER_CLOCK_HZ       16000000UL // Define la frecuencia base del sistema: 16 MHz (16 millones de ciclos por segundo)
#define TIM3_PSC_VAL         16         // Valor del preescalador para el Timer 3 (16MHz / 16 = 1MHz, es decir, 1 microsegundo por tick)

#define MIN_FREQUENCY        1000UL     // Límite inferior válido para la lectura (1 kHz)
#define MAX_FREQUENCY        55000UL    // Límite superior de lectura ampliado para abarcar hasta 99.9 kHz sin truncar
#define DISPLAY_DIGITS       5U         // Cantidad de displays de 7 segmentos conectados

// Configuración de la lógica de hardware para los componentes externos
#define SEGMENTS_ACTIVE_LOW  1 // Indica que los segmentos (LEDs) se encienden con un 0 lógico (GND)
#define COMMONS_ACTIVE_LOW   0 // Indica que los dígitos (transistores NPN) se activan con un 1 lógico (3.3V) en la base

// ================================================================
// PROTOTIPOS DE FUNCIONES
// ================================================================
void GPIO_Init(void);                      // Configura los pines de entrada/salida (Puertos A, B y C)
void TIM2_IC_Init(void);                   // Configura el Timer 2 en modo Input Capture para leer los flancos
void TIM3_Multiplex_Init(void);            // Configura el Timer 3 para refrescar los displays cada 2 milisegundos
void Update_Display_Digits(uint32_t freq); // Convierte el número entero de la frecuencia en 5 dígitos separados
void TIM2_IRQHandler(void);                // Rutina de interrupción del Timer 2 (Se ejecuta en cada flanco de subida)
void TIM3_IRQHandler(void);                // Rutina de interrupción del Timer 3 (Se ejecuta cada 2ms para el multiplexado)
int main(void);                            // Función principal donde inicia la ejecución

// ================================================================
// TABLA DE VECTORES DE INTERRUPCIÓN (BARE-METAL)
// ================================================================
extern uint32_t _estack; // Símbolo definido en el Linker Script que indica la dirección final de la memoria RAM (Stack Pointer)

void Reset_Handler(void) { main(); while (1); } // Primer código que corre al energizar; llama al main y tiene un bucle de seguridad
void Default_Handler(void) { while (1); }       // Trampa de seguridad si ocurre una interrupción no configurada

__attribute__((section(".isr_vector"))) // Coloca este arreglo exactamente al inicio de la memoria Flash (0x08000000)
void (* const g_pfnVectors[])(void) =
{
    (void (*)(void))(&_estack), // Posición 0: Dirección de inicio de la pila (Stack Pointer)
    Reset_Handler,              // Posición 1: Vector de Reset
    Default_Handler, Default_Handler, Default_Handler, Default_Handler, // Vectores de excepciones del sistema ARM Cortex-M4
    Default_Handler, Default_Handler, 0, 0, 0, 0,                       // Espacios reservados por la arquitectura
    Default_Handler, Default_Handler, 0, Default_Handler, Default_Handler, // Systick, PendSV, etc. (No usados, apuntan a Default)
    [16 + 28] = TIM2_IRQHandler, // Posición 44 (16 excepciones internas + IRQ 28): Vector del Timer 2
    [16 + 29] = TIM3_IRQHandler, // Posición 45 (16 excepciones internas + IRQ 29): Vector del Timer 3
};

// ================================================================
// VARIABLES GLOBALES
// ================================================================
volatile uint8_t current_digit = 0; // Lleva la cuenta de cuál de los 5 displays (0 a 4) se está encendiendo en el ciclo actual
volatile uint8_t digits_to_display[DISPLAY_DIGITS] = {0, 0, 0, 0, 0}; // Arreglo que guarda los números (0-9) a mostrar en cada dígito

// Variables para el cálculo del promedio por ventana
volatile uint64_t freq_sum = 0;       // Acumulador de 64 bits: Evita el desbordamiento al sumar miles de frecuencias altas
volatile uint32_t freq_count = 0;     // Cuenta cuántos ciclos válidos se han medido en el transcurso de 1 segundo
volatile uint32_t display_freq = 0;   // Guarda el valor de la frecuencia ya promediada que se enviará a la pantalla
volatile uint8_t one_second_flag = 0; // Bandera booleana (0 o 1) que avisa al bucle main que ya pasó un segundo

volatile uint32_t last_capture_tick = 0; // Almacena el valor del contador del Timer 2 en el instante del último flanco detectado

// ================================================================
// MACROS PARA CONTROL ATÓMICO DE PINES CON BSRR
// ================================================================
// El registro BSRR tiene 32 bits: los 16 bits bajos ponen el pin en 1 (3.3V), los 16 bits altos ponen el pin en 0 (GND)
#if SEGMENTS_ACTIVE_LOW
    #define SEG_OFF(pin) (1UL << (pin))        // Apaga el segmento mandando un 1 (3.3V iguala el voltaje del ánodo/cátodo)
    #define SEG_ON(pin)  (1UL << ((pin) + 16)) // Enciende el segmento mandando un 0 (GND permite el flujo de corriente)
#else
    #define SEG_OFF(pin) (1UL << ((pin) + 16))
    #define SEG_ON(pin)  (1UL << (pin))
#endif

#if COMMONS_ACTIVE_LOW
    #define COM_OFF(pin) (1UL << (pin))
    #define COM_ON(pin)  (1UL << ((pin) + 16))
#else
    #define COM_OFF(pin) (1UL << ((pin) + 16)) // Apaga el transistor NPN mandando un 0 (GND a la base del transistor)
    #define COM_ON(pin)  (1UL << (pin))        // Enciende el transistor NPN mandando un 1 (3.3V a la base del transistor)
#endif

// Mapa estándar de bits para crear números del 0 al 9 en un display de 7 segmentos (Bit0=A, Bit1=B ... Bit6=G)
const uint8_t segment_map[10] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

// Tabla pre-calculada de máscaras de 32 bits para el registro BSRR del Puerto C
// Cada línea define explícitamente qué pines (segmentos) se van a GND (ON) y cuáles a 3.3V (OFF) en un solo ciclo de reloj
const uint32_t segment_mask_complete[10] = {
    // 0: Enciende A(0), B(7), C(8), D(3), E(4), F(5)
    SEG_ON(0) | SEG_ON(7) | SEG_ON(8) | SEG_ON(3) | SEG_ON(4) | SEG_ON(5),
    // 1: Enciende B(7), C(8) | Apaga A(0), D(3), E(4), F(5)
    SEG_OFF(0) | SEG_ON(7) | SEG_ON(8) | SEG_OFF(3) | SEG_OFF(4) | SEG_OFF(5),
    // 2: Enciende A, B, D, E | Apaga C, F
    SEG_ON(0) | SEG_ON(7) | SEG_OFF(8) | SEG_ON(3) | SEG_ON(4) | SEG_OFF(5),
    // 3: Enciende A, B, C, D | Apaga E, F
    SEG_ON(0) | SEG_ON(7) | SEG_ON(8) | SEG_ON(3) | SEG_OFF(4) | SEG_OFF(5),
    // 4: Enciende B, C, F | Apaga A, D, E
    SEG_OFF(0) | SEG_ON(7) | SEG_ON(8) | SEG_OFF(3) | SEG_OFF(4) | SEG_ON(5),
    // 5: Enciende A, C, D, F | Apaga B, E
    SEG_ON(0) | SEG_OFF(7) | SEG_ON(8) | SEG_ON(3) | SEG_OFF(4) | SEG_ON(5),
    // 6: Enciende A, C, D, E, F | Apaga B
    SEG_ON(0) | SEG_OFF(7) | SEG_ON(8) | SEG_ON(3) | SEG_ON(4) | SEG_ON(5),
    // 7: Enciende A, B, C | Apaga D, E, F
    SEG_ON(0) | SEG_ON(7) | SEG_ON(8) | SEG_OFF(3) | SEG_OFF(4) | SEG_OFF(5),
    // 8: Enciende A, B, C, D, E, F
    SEG_ON(0) | SEG_ON(7) | SEG_ON(8) | SEG_ON(3) | SEG_ON(4) | SEG_ON(5),
    // 9: Enciende A, B, C, D, F | Apaga E
    SEG_ON(0) | SEG_ON(7) | SEG_ON(8) | SEG_ON(3) | SEG_OFF(4) | SEG_ON(5)
};

// ================================================================
// CONFIGURACIÓN DE PINES (GPIO)
// ================================================================
void GPIO_Init(void)
{
    // Habilita el reloj del bus AHB1 para energizar los puertos A, B y C
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN;

    // Configuración del pin PA0 (Entrada de la señal del generador conectada a TIM2_CH1)
    GPIOA->MODER &= ~(3UL << 0); // Limpia los bits 0 y 1 del MODER para el pin PA0
    GPIOA->MODER |= (2UL << 0);  // Establece el modo a '10' (Función Alternativa)
    GPIOA->AFR[0] &= ~(0xFUL << 0); // Limpia los 4 bits del Alternate Function Register Low (AFRL) para PA0
    GPIOA->AFR[0] |= (1UL << 0); // Asigna la Función Alternativa 1 (AF1) que conecta PA0 internamente al Timer 2
    GPIOA->PUPDR &= ~(3UL << 0); // Limpia las resistencias internas de Pull-up/Pull-down del pin PA0
    GPIOA->PUPDR |= (2UL << 0);  // Activa la resistencia Pull-down interna para evitar lecturas flotantes cuando no hay señal

    // Configuración de los pines PC0, PC3, PC4, PC5, PC7, PC8 (Salidas para los segmentos A-F)
    uint32_t mask_c = (3UL << 0) | (3UL << 6) | (3UL << 8) | (3UL << 10) | (3UL << 14) | (3UL << 16); // Máscara de limpieza para los pines
    GPIOC->MODER &= ~mask_c; // Borra configuraciones previas de esos pines
    GPIOC->MODER |= (1UL << 0) | (1UL << 6) | (1UL << 8) | (1UL << 10) | (1UL << 14) | (1UL << 16); // Los pone en modo '01' (Salida General Push-Pull)

    // Configuración del pin PA9 (Salida para el segmento G)
    GPIOA->MODER &= ~(3UL << 18); // Limpia configuración del pin 9 (2 bits por pin = desplazamiento 18)
    GPIOA->MODER |= (1UL << 18);  // Modo de salida '01'

    // Configuración de los pines PB0 a PB4 (Salidas para activar las bases de los transistores de cada dígito)
    GPIOB->MODER &= ~0x000003FF; // Limpia configuración de los pines del 0 al 4
    GPIOB->MODER |= 0x00000155;  // Asigna '01' a los primeros 5 pines (Modo de salida)

    // Inicialización segura: Asegura que todo esté apagado antes de que el Timer 3 empiece a funcionar
    GPIOB->BSRR = COM_OFF(0) | COM_OFF(1) | COM_OFF(2) | COM_OFF(3) | COM_OFF(4); // Apaga los 5 transistores
    GPIOC->BSRR = SEG_OFF(0) | SEG_OFF(3) | SEG_OFF(4) | SEG_OFF(5) | SEG_OFF(7) | SEG_OFF(8); // Apaga segmentos A-F
    GPIOA->BSRR = SEG_OFF(9); // Apaga el segmento G
}

// ================================================================
// CONFIGURACIÓN TIM2 (MEDICIÓN DE FRECUENCIA - INPUT CAPTURE)
// ================================================================
void TIM2_IC_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // Enciende el reloj del Timer 2 en el bus APB1
    TIM2->PSC = 0;          // Preescaler en 0: El timer corre a la velocidad máxima del sistema (16 MHz)
    TIM2->ARR = 0xFFFFFFFF; // Auto-Reload Register al máximo valor de 32 bits para que cuente sin reiniciarse prematuramente

    // Configuración del canal de captura
    TIM2->CCMR1 &= ~(3UL << 0); // Limpia la selección del canal
    TIM2->CCMR1 |= (1UL << 0);  // Asigna la entrada TI1 al Canal 1 (CC1S = 01)

    // Configuración de filtro de hardware (muy útil para evitar ruidos parásitos en la protoboard)
    TIM2->CCMR1 &= ~(0xFUL << 4); // Limpia los bits del filtro
    TIM2->CCMR1 |= (4UL << 4);    // IC1F = 0100: Requiere que la señal se mantenga estable por 8 ciclos de reloj para validar el flanco

    TIM2->CCER &= ~((1UL << 1) | (1UL << 3)); // CC1P=0 y CC1NP=0: Configura el hardware para detectar flancos de SUBIDA
    TIM2->CCER |= (1UL << 0);  // CC1E = 1: Habilita finalmente la captura en el Canal 1

    TIM2->DIER |= TIM_DIER_CC1IE; // Habilita la interrupción que avisa cuando ocurrió una captura (flanco detectado)
    NVIC_EnableIRQ(28); // Le dice al núcleo ARM que escuche las interrupciones del Timer 2 (Posición 28 en el NVIC)
    
    TIM2->CNT = 0; // Reinicia el contador principal a cero
    TIM2->SR = 0;  // Limpia cualquier bandera de interrupción colgada
    TIM2->CR1 |= TIM_CR1_CEN; // Enciende el contador del Timer 2
}

// ================================================================
// CONFIGURACIÓN TIM3 (MULTIPLEXACIÓN DE PANTALLA)
// ================================================================
void TIM3_Multiplex_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; // Enciende el reloj del Timer 3

    TIM3->PSC = TIM3_PSC_VAL - 1; // Divide el reloj de 16MHz entre 16, resultando en ticks de 1 microsegundo
    TIM3->ARR = 2000 - 1;         // Cuenta hasta 2000 ticks (2 milisegundos) y genera una interrupción de reinicio

    TIM3->DIER |= TIM_DIER_UIE; // Habilita la interrupción por Update (desbordamiento del ARR)
    TIM3->SR = 0; // Limpia banderas
    NVIC_EnableIRQ(29); // Habilita interrupción del Timer 3 en el NVIC (Posición 29)
    TIM3->CR1 |= TIM_CR1_CEN; // Enciende el contador del Timer 3
}

// ================================================================
// FUNCIÓN PRINCIPAL
// ================================================================
int main(void)
{
    GPIO_Init();           // Ejecuta la inicialización de pines
    TIM2_IC_Init();        // Ejecuta la inicialización del hardware de medición
    TIM3_Multiplex_Init(); // Arranca el motor de multiplexado visual

    display_freq = 0; // Inicializa la variable de visualización en 0
    Update_Display_Digits(display_freq); // Separa el '0' en el arreglo para que las pantallas muestren 00000

    while (1) // Bucle infinito principal
    {
        // El bucle principal solo reacciona cuando el Timer 3 levanta la bandera de 1 segundo cumplido
        if (one_second_flag)
        {
            one_second_flag = 0; // Baja la bandera inmediatamente tras detectarla
            Update_Display_Digits(display_freq); // Actualiza el arreglo con la nueva frecuencia para cambiar lo que se ve
        }
    }
}

// ================================================================
// RUTINA DE INTERRUPCIÓN DEL TIM2 (MEDICIÓN FÍSICA)
// ================================================================
void TIM2_IRQHandler(void)
{
    if (TIM2->SR & (1UL << 1)) // Si la bandera de captura del canal 1 (CC1IF) está arriba (ocurrió un flanco)
    {
        uint32_t current_tick = TIM2->CCR1; // Lee y guarda en qué número exacto del contador ocurrió el flanco
        uint32_t period_ticks = current_tick - last_capture_tick; // Calcula la distancia en ciclos desde el flanco anterior (El periodo T)
        last_capture_tick = current_tick; // Guarda el valor actual como el "último" para la próxima medición

        // Filtro pasabajos por software: Ignora periodos menores a 150 ticks. 
        // 16,000,000 / 150 = 106,666 Hz. Cualquier pulso más rápido que eso se asume como ruido electromagnético.
        if (period_ticks > 150) 
        {
            // Calcula la frecuencia real de esta muestra específica: f = Frecuencia del reloj / ticks del periodo
            uint32_t calc_freq = TIMER_CLOCK_HZ / period_ticks;
            
            // Verifica que la señal caiga dentro del rango válido de tu laboratorio (1kHz a 100kHz)
            if (calc_freq >= MIN_FREQUENCY && calc_freq <= MAX_FREQUENCY) {
                freq_sum += calc_freq; // Acumula el valor en la variable gigante de 64 bits
                freq_count++;          // Incrementa el contador de muestras válidas tomadas en este segundo
            }
        }
        TIM2->SR &= ~(1UL << 1); // Baja la bandera de interrupción para poder atender el siguiente flanco
    }
}

// ================================================================
// RUTINA DE INTERRUPCIÓN DEL TIM3 (REFRESCO DE PANTALLA Y RELOJ)
// ================================================================
void TIM3_IRQHandler(void)
{
    if (TIM3->SR & (1UL << 0)) // Si la interrupción fue por Update (pasaron 2 milisegundos)
    {
        // PASO 1: Para evitar que el dígito anterior se "embarre" en el nuevo (ghosting), apagamos todos los transistores primero
        GPIOB->BSRR = COM_OFF(0) | COM_OFF(1) | COM_OFF(2) | COM_OFF(3) | COM_OFF(4);

        // PASO 2: Obtener el número que toca mostrar en este turno desde el arreglo
        uint8_t val = digits_to_display[current_digit];
        
        if (val > 9) val = 0; // Protección crítica: Si por error de memoria hay un 15, lo baja a 0 para no desbordar arreglos
        
        // Aplica el patrón de segmentos entero para los puertos de C en un solo golpe
        GPIOC->BSRR = segment_mask_complete[val];
        
        // El segmento G está en otro puerto (PA9), así que se evalúa por separado usando la máscara básica hexadecimal 0x40 (bit 6)
        if (segment_map[val] & 0x40) {
            GPIOA->BSRR = SEG_ON(9); // Si el bit 6 es 1, encender el segmento G
        } else {
            GPIOA->BSRR = SEG_OFF(9); // Si no, apagarlo
        }

        // PASO 3: Ahora que los segmentos están listos con la nueva forma, encendemos exclusivamente el transistor del dígito actual
        GPIOB->BSRR = COM_ON(current_digit);

        // PASO 4: Incrementamos el índice para que en los próximos 2ms se encienda la pantalla siguiente
        current_digit++;
        if (current_digit >= DISPLAY_DIGITS) current_digit = 0; // Si ya pintamos el quinto dígito (índice 4), vuelve al primero (índice 0)

        // PASO 5: Bloque de temporización (1 Segundo) y cálculo de promedio final
        // Esta interrupción se ejecuta cada 2ms. Si contamos hasta 500 interrupciones, sabemos que ha pasado exactamente 1 segundo (500 * 2 = 1000ms)
        static uint16_t irq_count = 0; // Variable estática que no se borra cuando la función termina
        irq_count++;
        if (irq_count >= 500) // Se cumplió 1 segundo
        {
            irq_count = 0; // Reinicia el contador de tiempo
            
            uint32_t current_time = TIM2->CNT; // Chequea dónde está el contador del hardware de medición
            // TIMEOUT: Si el contador de medición se distanció más de 16,000,000 de ciclos de la última lectura, significa que hace más de 1 segundo que no entra señal
            if ((current_time - last_capture_tick) > 16000000UL) {
                display_freq = 0; // Fuerza la pantalla a mostrar 0 porque el cable está desconectado o la fuente se apagó
            } else if (freq_count > 0) {
                // Si hubo capturas válidas, calcula el promedio matemático dividiendo la suma gigante entre la cantidad de muestras, y lo corta a 32 bits
                display_freq = (uint32_t)(freq_sum / freq_count);
            } 
            
            // Borrón y cuenta nueva: Se vacían los acumuladores para empezar a medir el próximo segundo
            freq_sum = 0;
            freq_count = 0;
            
            // Activa la señal para que el bucle principal sepa que ya hay un dato nuevo cocinado y lo divida
            one_second_flag = 1;
        }

        TIM3->SR &= ~(1UL << 0); // Limpia la bandera del Timer 3 para permitir la próxima interrupción
    }
}

// ================================================================
// EXTRACCIÓN Y ORDENAMIENTO DE DÍGITOS
// ================================================================
void Update_Display_Digits(uint32_t freq)
{
    if (freq > 50000) freq = 50000; // Tope visual: Como solo hay 5 displays, no podemos mostrar números de 6 cifras. Se trunca en 99,999.
    
    // Extracción matemática de posiciones mediante divisiones enteras y residuos (módulo 10)
    // El orden está ajustado a tu hardware: PB0 (índice 0) va conectado al dígito de la izquierda del bloque de 4 (Decenas de mil).
    // PB4 (índice 4) va conectado a tu display suelto individual (Unidades).
    digits_to_display[0] = (freq / 10000) % 10; // Extrae el dígito de las decenas de millar (Ej. En 49930 saca el 4)
    digits_to_display[1] = (freq / 1000) % 10;  // Extrae las unidades de millar (Ej. En 49930 saca el 9)
    digits_to_display[2] = (freq / 100) % 10;   // Extrae las centenas (Ej. En 49930 saca el 9)
    digits_to_display[3] = (freq / 10) % 10;    // Extrae las decenas (Ej. En 49930 saca el 3)
    digits_to_display[4] = freq % 10;           // Extrae el último número directo (Ej. En 49930 saca el 0 para el display individual)
}