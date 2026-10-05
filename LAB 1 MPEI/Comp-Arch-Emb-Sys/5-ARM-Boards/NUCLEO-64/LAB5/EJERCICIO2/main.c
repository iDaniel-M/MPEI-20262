#include "stm32f446xx.h" // Importa el mapa de memoria y registros especIficos del microcontrolador STM32F446RE
#include <stdint.h>      // Permite usar tipos de datos de tamaño fijo estandarizados como uint32_t, uint8_t, etc.

 
// CONFIGURACION DEL SISTEMA
 

#define TIMER_CLOCK_HZ       16000000UL // Define la frecuencia base del sistema: 16 MHz
#define TIM3_PSC_VAL         16         // Valor del preescalador para el Timer 3 (16MHz / 16 = 1MHz, es decir, 1 microsegundo por tick)

#define MIN_FREQUENCY        1000UL     // Limite inferior valido para la lectura (1 kHz)
#define MAX_FREQUENCY        55000UL    // Limite superior de lectura
#define DISPLAY_DIGITS       5U         // Cantidad de displays de 7 segmentos conectados

// ConfiguraciOn de la lOgica de hardware para los componentes externos
#define SEGMENTS_ACTIVE_LOW  1 // Indica que los segmentos (LEDs) se encienden con un 0 lOgico (GND)
#define COMMONS_ACTIVE_LOW   0 // Indica que los dIgitos (transistores NPN) se activan con un 1 lOgico (3.3V) en la base

 
// PROTOTIPOS DE FUNCIONES
 
void GPIO_Init(void);                    // Configura los pines de entrada/salida (Puertos A, B y C)
void TIM2_IC_Init(void);                 // Configura el Timer 2 en modo Input Capture para leer los flancos
void TIM3_Multiplex_Init(void);            // Configura el Timer 3 para refrescar los displays cada 2 milisegundos
void Update_Display_Digits(uint32_t freq); // Convierte el nUmero entero de la frecuencia en 5 dIgitos separados
void TIM2_IRQHandler(void);                // Rutina de interrupciOn del Timer 2 (Se ejecuta en cada flanco de subida)
void TIM3_IRQHandler(void);                // Rutina de interrupciOn del Timer 3 (Se ejecuta cada 2ms para el multiplexado)
int main(void);                            // FunciOn principal donde inicia la ejecuciOn

 
// TABLA DE VECTORES DE INTERRUPCION (BARE-METAL)
 
extern uint32_t _estack; // SImbolo definido en el Linker Script que indica la direcciOn final de la memoria RAM (Stack Pointer)

void Reset_Handler(void) { main(); while (1); } // Primer cOdigo que corre al energizar; llama al main y tiene un bucle de seguridad
void Default_Handler(void) { while (1); }       // Trampa de seguridad si ocurre una interrupciOn no configurada

__attribute__((section(".isr_vector"))) // Coloca este arreglo exactamente al inicio de la memoria Flash (0x08000000)
void (* const g_pfnVectors[])(void) =
{
    (void (*)(void))(&_estack), // PosiciOn 0: DirecciOn de inicio de la pila (Stack Pointer)
    Reset_Handler,              // PosiciOn 1: Vector de Reset
    Default_Handler, Default_Handler, Default_Handler, Default_Handler, // Vectores de excepciones del sistema ARM Cortex-M4
    Default_Handler, Default_Handler, 0, 0, 0, 0,                       // Espacios reservados por la arquitectura
    Default_Handler, Default_Handler, 0, Default_Handler, Default_Handler, // Systick, PendSV, etc. (No usados, apuntan a Default)
    [16 + 28] = TIM2_IRQHandler, // PosiciOn 44 (16 excepciones internas + IRQ 28): Vector del Timer 2
    [16 + 29] = TIM3_IRQHandler, // PosiciOn 45 (16 excepciones internas + IRQ 29): Vector del Timer 3
};

 
// VARIABLES GLOBALES
 
volatile uint8_t current_digit = 0; // Lleva la cuenta de cuAl de los 5 displays (0 a 4) se estA encendiendo en el ciclo actual
volatile uint8_t digits_to_display[DISPLAY_DIGITS] = {0, 0, 0, 0, 0}; // Arreglo que guarda los nUmeros (0-9) a mostrar en cada dIgito

// Variables para el cAlculo del promedio por ventana
volatile uint64_t freq_sum = 0;       // Acumulador de 64 bits: Evita el desbordamiento al sumar miles de frecuencias altas
volatile uint32_t freq_count = 0;     // Cuenta cuAntos ciclos vAlidos se han medido en el transcurso de 1 segundo
volatile uint32_t display_freq = 0;   // Guarda el valor de la frecuencia ya promediada que se enviarA a la pantalla
volatile uint8_t one_second_flag = 0; // Bandera booleana (0 o 1) que avisa al bucle main que ya pasO un segundo

volatile uint32_t last_capture_tick = 0; // Almacena el valor del contador del Timer 2 en el instante del Ultimo flanco detectado

 
// MACROS PARA CONTROL ATOMICO DE PINES CON BSRR
 
// El registro BSRR tiene 32 bits: los 16 bits bajos ponen el pin en 1 (3.3V), los 16 bits altos ponen el pin en 0 (GND)
#if SEGMENTS_ACTIVE_LOW
    #define SEG_OFF(pin) (1UL << (pin))        // Apaga el segmento mandando un 1 (3.3V iguala el voltaje del Anodo/cAtodo)
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

// Mapa estAndar de bits para crear nUmeros del 0 al 9 en un display de 7 segmentos (Bit0=A, Bit1=B ... Bit6=G)
const uint8_t segment_map[10] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

// Tabla pre-calculada de mAscaras de 32 bits para el registro BSRR del Puerto C
// Cada lInea define explIcitamente quE pines (segmentos) se van a GND (ON) y cuAles a 3.3V (OFF) en un solo ciclo de reloj
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

 
// CONFIGURACION DE PINES (GPIO)
 
void GPIO_Init(void)
{
    // Habilita el reloj del bus AHB1 para energizar los puertos A, B y C
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN;

    // ConfiguraciOn del pin PA0 (Entrada de la señal del generador conectada a TIM2_CH1)
    GPIOA->MODER &= ~(3UL << 0); // Limpia los bits 0 y 1 del MODER para el pin PA0
    GPIOA->MODER |= (2UL << 0);  // Establece el modo a '10' (FunciOn Alternativa)
    GPIOA->AFR[0] &= ~(0xFUL << 0); // Limpia los 4 bits del Alternate Function Register Low (AFRL) para PA0
    GPIOA->AFR[0] |= (1UL << 0); // Asigna la FunciOn Alternativa 1 (AF1) que conecta PA0 internamente al Timer 2
    GPIOA->PUPDR &= ~(3UL << 0); // Limpia las resistencias internas de Pull-up/Pull-down del pin PA0
    GPIOA->PUPDR |= (2UL << 0);  // Activa la resistencia Pull-down interna para evitar lecturas flotantes cuando no hay señal

    // ConfiguraciOn de los pines PC0, PC3, PC4, PC5, PC7, PC8 (Salidas para los segmentos A-F)
    uint32_t mask_c = (3UL << 0) | (3UL << 6) | (3UL << 8) | (3UL << 10) | (3UL << 14) | (3UL << 16); // MAscara de limpieza para los pines
    GPIOC->MODER &= ~mask_c; // Borra configuraciones previas de esos pines
    GPIOC->MODER |= (1UL << 0) | (1UL << 6) | (1UL << 8) | (1UL << 10) | (1UL << 14) | (1UL << 16); // Los pone en modo '01' (Salida General Push-Pull)

    // ConfiguraciOn del pin PA9 (Salida para el segmento G)
    GPIOA->MODER &= ~(3UL << 18); // Limpia configuraciOn del pin 9 (2 bits por pin = desplazamiento 18)
    GPIOA->MODER |= (1UL << 18);  // Modo de salida '01'

    // ConfiguraciOn de los pines PB0 a PB4 (Salidas para activar las bases de los transistores de cada dIgito)
    GPIOB->MODER &= ~0x000003FF; // Limpia configuraciOn de los pines del 0 al 4
    GPIOB->MODER |= 0x00000155;  // Asigna '01' a los primeros 5 pines (Modo de salida)

    // InicializaciOn segura: Asegura que todo estE apagado antes de que el Timer 3 empiece a funcionar
    GPIOB->BSRR = COM_OFF(0) | COM_OFF(1) | COM_OFF(2) | COM_OFF(3) | COM_OFF(4); // Apaga los 5 transistores
    GPIOC->BSRR = SEG_OFF(0) | SEG_OFF(3) | SEG_OFF(4) | SEG_OFF(5) | SEG_OFF(7) | SEG_OFF(8); // Apaga segmentos A-F
    GPIOA->BSRR = SEG_OFF(9); // Apaga el segmento G
}

 
// CONFIGURACION TIM2 (MEDICION DE FRECUENCIA - INPUT CAPTURE)
 
void TIM2_IC_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // Enciende el reloj del Timer 2 en el bus APB1
    TIM2->PSC = 0;          // Preescaler en 0: El timer corre a la velocidad mAxima del sistema (16 MHz)
    TIM2->ARR = 0xFFFFFFFF; // Auto-Reload Register al mAximo valor de 32 bits para que cuente sin reiniciarse prematuramente

    // ConfiguraciOn del canal de captura
    TIM2->CCMR1 &= ~(3UL << 0); // Limpia la selecciOn del canal
    TIM2->CCMR1 |= (1UL << 0);  // Asigna la entrada TI1 al Canal 1 (CC1S = 01)

    // ConfiguraciOn de filtro de hardware (muy Util para evitar ruidos parAsitos en la protoboard)
    TIM2->CCMR1 &= ~(0xFUL << 4); // Limpia los bits del filtro
    TIM2->CCMR1 |= (4UL << 4);    // IC1F = 0100: Requiere que la señal se mantenga estable por 8 ciclos de reloj para validar el flanco

    TIM2->CCER &= ~((1UL << 1) | (1UL << 3)); // CC1P=0 y CC1NP=0: Configura el hardware para detectar flancos de SUBIDA
    TIM2->CCER |= (1UL << 0);  // CC1E = 1: Habilita finalmente la captura en el Canal 1

    TIM2->DIER |= TIM_DIER_CC1IE; // Habilita la interrupciOn que avisa cuando ocurriO una captura (flanco detectado)
    NVIC_EnableIRQ(28); // Le dice al nUcleo ARM que escuche las interrupciones del Timer 2 (PosiciOn 28 en el NVIC)
    
    TIM2->CNT = 0; // Reinicia el contador principal a cero
    TIM2->SR = 0;  // Limpia cualquier bandera de interrupciOn colgada
    TIM2->CR1 |= TIM_CR1_CEN; // Enciende el contador del Timer 2
}

// CONFIGURACION TIM3 (MULTIPLEXACION DE PANTALLA)

void TIM3_Multiplex_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; // Enciende el reloj del Timer 3

    TIM3->PSC = TIM3_PSC_VAL - 1; // Divide el reloj de 16MHz entre 16, resultando en ticks de 1 microsegundo
    TIM3->ARR = 2000 - 1;         // Cuenta hasta 2000 ticks (2 milisegundos) y genera una interrupciOn de reinicio

    TIM3->DIER |= TIM_DIER_UIE; // Habilita la interrupciOn por Update (desbordamiento del ARR)
    TIM3->SR = 0; // Limpia banderas
    NVIC_EnableIRQ(29); // Habilita interrupciOn del Timer 3 en el NVIC (PosiciOn 29)
    TIM3->CR1 |= TIM_CR1_CEN; // Enciende el contador del Timer 3
}

 
// FUNCION PRINCIPAL
 
int main(void)
{
    GPIO_Init();           // Ejecuta la inicializaciOn de pines
    TIM2_IC_Init();        // Ejecuta la inicializaciOn del hardware de mediciOn
    TIM3_Multiplex_Init(); // Arranca el motor de multiplexado visual

    display_freq = 0; // Inicializa la variable de visualizaciOn en 0
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


// RUTINA DE INTERRUPCION DEL TIM2 (MEDICION FISICA)
void TIM2_IRQHandler(void)
{
    if (TIM2->SR & (1UL << 1)) // Si la bandera de captura del canal 1 (CC1IF) estA arriba (ocurriO un flanco)
    {
        uint32_t current_tick = TIM2->CCR1; // Lee y guarda en quE nUmero exacto del contador ocurriO el flanco
        uint32_t period_ticks = current_tick - last_capture_tick; // Calcula la distancia en ciclos desde el flanco anterior (El periodo T)
        last_capture_tick = current_tick; // Guarda el valor actual como el "Ultimo" para la prOxima mediciOn

        // Filtro pasabajos por software: Ignora periodos menores a 150 ticks. 
        // 16,000,000 / 150 = 106,666 Hz. Cualquier pulso mAs rApido que eso se asume como ruido electromagnEtico.
        if (period_ticks > 150) 
        {
            // Calcula la frecuencia real de esta muestra especIfica: f = Frecuencia del reloj / ticks del periodo
            uint32_t calc_freq = TIMER_CLOCK_HZ / period_ticks;
            
            // Verifica que la señal caiga dentro del rango vAlido de tu laboratorio (1kHz a 100kHz)
            if (calc_freq >= MIN_FREQUENCY && calc_freq <= MAX_FREQUENCY) {
                freq_sum += calc_freq; // Acumula el valor en la variable gigante de 64 bits
                freq_count++;          // Incrementa el contador de muestras vAlidas tomadas en este segundo
            }
        }
        TIM2->SR &= ~(1UL << 1); // Baja la bandera de interrupciOn para poder atender el siguiente flanco
    }
}

 
// RUTINA DE INTERRUPCION DEL TIM3 (REFRESCO DE PANTALLA Y RELOJ)
 
void TIM3_IRQHandler(void)
{
    if (TIM3->SR & (1UL << 0)) // Si la interrupciOn fue por Update (pasaron 2 milisegundos)
    {
        // PASO 1: Para evitar que el dIgito anterior se "embarre" en el nuevo (ghosting), apagamos todos los transistores primero
        GPIOB->BSRR = COM_OFF(0) | COM_OFF(1) | COM_OFF(2) | COM_OFF(3) | COM_OFF(4);

        // PASO 2: Obtener el nUmero que toca mostrar en este turno desde el arreglo
        uint8_t val = digits_to_display[current_digit];
        
        if (val > 9) val = 0; // ProtecciOn crItica: Si por error de memoria hay un 15, lo baja a 0 para no desbordar arreglos
        
        // Aplica el patrOn de segmentos entero para los puertos de C en un solo golpe
        GPIOC->BSRR = segment_mask_complete[val];
        
        // El segmento G estA en otro puerto (PA9), asI que se evalUa por separado usando la mAscara bAsica hexadecimal 0x40 (bit 6)
        if (segment_map[val] & 0x40) {
            GPIOA->BSRR = SEG_ON(9); // Si el bit 6 es 1, encender el segmento G
        } else {
            GPIOA->BSRR = SEG_OFF(9); // Si no, apagarlo
        }

        // PASO 3: Ahora que los segmentos estAn listos con la nueva forma, encendemos exclusivamente el transistor del dIgito actual
        GPIOB->BSRR = COM_ON(current_digit);

        // PASO 4: Incrementamos el Indice para que en los prOximos 2ms se encienda la pantalla siguiente
        current_digit++;
        if (current_digit >= DISPLAY_DIGITS) current_digit = 0; // Si ya pintamos el quinto dIgito (Indice 4), vuelve al primero (Indice 0)

        // PASO 5: Bloque de temporizaciOn (1 Segundo) y cAlculo de promedio final
        // Esta interrupciOn se ejecuta cada 2ms. Si contamos hasta 500 interrupciones, sabemos que ha pasado exactamente 1 segundo (500 * 2 = 1000ms)
        static uint16_t irq_count = 0; // Variable estAtica que no se borra cuando la funciOn termina
        irq_count++;
        if (irq_count >= 500) // Se cumpliO 1 segundo
        {
            irq_count = 0; // Reinicia el contador de tiempo
            
            uint32_t current_time = TIM2->CNT; // Chequea dOnde estA el contador del hardware de mediciOn
            // TIMEOUT: Si el contador de mediciOn se distanciO mAs de 16,000,000 de ciclos de la Ultima lectura, significa que hace mAs de 1 segundo que no entra señal
            if ((current_time - last_capture_tick) > 16000000UL) {
                display_freq = 0; // Fuerza la pantalla a mostrar 0 porque el cable estA desconectado o la fuente se apagO
            } else if (freq_count > 0) {
                // Si hubo capturas vAlidas, calcula el promedio matemAtico dividiendo la suma gigante entre la cantidad de muestras, y lo corta a 32 bits
                display_freq = (uint32_t)(freq_sum / freq_count);
            } 
            
            // BorrOn y cuenta nueva: Se vacIan los acumuladores para empezar a medir el prOximo segundo
            freq_sum = 0;
            freq_count = 0;
            
            // Activa la señal para que el bucle principal sepa que ya hay un dato nuevo cocinado y lo divida
            one_second_flag = 1;
        }

        TIM3->SR &= ~(1UL << 0); // Limpia la bandera del Timer 3 para permitir la prOxima interrupciOn
    }
}

 
// EXTRACCION Y ORDENAMIENTO DE DIGITOS
 
void Update_Display_Digits(uint32_t freq)
{
    if (freq > 50000) freq = 50000; // Tope visual: Como solo hay 5 displays, no podemos mostrar nUmeros de 6 cifras. Se trunca en 99,999.
    
    // ExtracciOn matemAtica de posiciones mediante divisiones enteras y residuos (mOdulo 10)
    // El orden estA ajustado a tu hardware: PB0 (Indice 0) va conectado al dIgito de la izquierda del bloque de 4 (Decenas de mil).
    // PB4 (Indice 4) va conectado a tu display suelto individual (Unidades).
    digits_to_display[0] = (freq / 10000) % 10; // Extrae el dIgito de las decenas de millar (Ej. En 49930 saca el 4)
    digits_to_display[1] = (freq / 1000) % 10;  // Extrae las unidades de millar (Ej. En 49930 saca el 9)
    digits_to_display[2] = (freq / 100) % 10;   // Extrae las centenas (Ej. En 49930 saca el 9)
    digits_to_display[3] = (freq / 10) % 10;    // Extrae las decenas (Ej. En 49930 saca el 3)
    digits_to_display[4] = freq % 10;           // Extrae el Ultimo nUmero directo (Ej. En 49930 saca el 0 para el display individual)
}