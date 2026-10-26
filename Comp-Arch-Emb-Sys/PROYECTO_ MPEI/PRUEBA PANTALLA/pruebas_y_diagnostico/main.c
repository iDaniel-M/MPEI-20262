/*
 * main.c - PRUEBA DE PANTALLA P10 RGB (HUB75, escaneo 1/4) con STM32F446RE
 *
 * Objetivo: comprobar el cableado y DESCUBRIR el mapeo interno del panel
 * (qué bit de la cadena de desplazamiento corresponde a qué LED).
 *
 * Reloj: HSI interno de 16 MHz (el que queda tras reset, sin PLL).
 *
 * Conexiones (HUB75 IN del panel 1 -> Nucleo):
 *   R1=PC0  G1=PC1  B1=PC2  R2=PC3  G2=PC4  B2=PC5
 *   OE=PC6 (TIM3_CH1, PWM por hardware)   LAT=PC7   CLK=PC8
 *   A=PC10  B=PC11   GND=GND
 * Otros: LED verde PA5 (latido), botón azul PC13 (cambia de modo),
 *        USART2 TX PA2 -> Virtual COM del ST-LINK, 115200 8N1.
 *
 * MODOS (se cambian con el botón azul):
 *   0 WALK : un solo bit encendido que recorre toda la cadena, fase por fase.
 *            Arriba (R1) en ROJO, abajo (R2..B2) en AZUL. Por UART se imprime
 *            "fase=F bit=K" para anotar dónde aparece cada LED.
 *   1 FASE : enciende todos los bits de UNA fase (A/B fijos). Muestra qué filas
 *            maneja cada combinación de A y B.
 *   2 FILL : pantalla completa en rojo, verde y azul (brillo bajo).
 */
#include <stdint.h>
#include "regs.h"

/* ======================= Parámetros de la prueba ======================= */
#ifndef N_PANELS
#define N_PANELS 1            /* paneles en cadena (se puede pasar desde el Makefile) */
#endif
#define BITS_PER_PANEL 64     /* 32 columnas x 2 filas por línea de dato en 1/4 scan */
#define NCLK (BITS_PER_PANEL * N_PANELS)   /* pulsos de CLK por fase */
#define N_PHASES 4            /* escaneo 1/4 -> A,B seleccionan 4 fases */

#define USE_FM6126A_INIT 1    /* 1 = enviar la configuración de drivers FM6126A/ICN2038S al arrancar.
                                 Sin ella esos chips dejan las salidas apagadas. En drivers
                                 comunes (ICN2037, 595...) no hace daño: solo carga datos. */
#define OE_ACTIVE_LOW 1       /* HUB75 típico: OE=0 enciende. Si todo queda al revés, poner 0 */
#define DATA_ACTIVE_HIGH 1    /* 1 en R/G/B = LED encendido. Si se ve en negativo, poner 0 */

/* Timer de refresco: 16 MHz / (PSC+1) = 1 MHz -> 1 tick = 1 us
   Período de cada fase = ARR+1 = 500 us -> 2000 fases/s -> 500 cuadros/s */
#define TIM_PSC     15u
#define PHASE_US    500u
#define TICKS_PER_S 2000u     /* interrupciones de TIM3 por segundo */

/* ======================= Pines (todos en GPIOC) ======================= */
#define PIN_R1  0
#define PIN_G1  1
#define PIN_B1  2
#define PIN_R2  3
#define PIN_G2  4
#define PIN_B2  5
#define PIN_OE  6
#define PIN_LAT 7
#define PIN_CLK 8
#define PIN_A   10
#define PIN_B   11

#define SET(p) (1u << (p))           /* para BSRR: poner pin en 1 */
#define RST(p) (1u << ((p) + 16))    /* para BSRR: poner pin en 0 */

/* Bits de color tal como quedan en GPIOC (bit n = PCn) */
#define C_R1 (1u << PIN_R1)
#define C_G1 (1u << PIN_G1)
#define C_B1 (1u << PIN_B1)
#define C_R2 (1u << PIN_R2)
#define C_G2 (1u << PIN_G2)
#define C_B2 (1u << PIN_B2)
#define DATA_MASK 0x3Fu              /* PC0..PC5 */

/* ======================= Buffer crudo =======================
 * raw[f][k] = valor de las 6 líneas de dato en el pulso de CLK k de la fase f.
 * Se guarda "crudo" (orden de la cadena) a propósito: en esta prueba aún no
 * sabemos cómo se mapea a (x,y); eso es justamente lo que vamos a medir. */
static volatile uint8_t raw[N_PHASES][NCLK];

static volatile uint32_t ticks;          /* +1 en cada interrupción (0.5 ms) */
static volatile uint8_t  shifted_phase;  /* fase cuyos datos ya están en los registros */

/* ======================= Refresco HUB75 =======================
 * Desplaza NCLK columnas por las 6 líneas en paralelo.
 * Por columna: 1 escritura pone los datos y baja CLK, otra sube CLK
 * (el dato se captura en el flanco de subida). BSRR es atómico: no hace
 * falta leer-modificar-escribir ODR. */
static void shift_phase(uint8_t f)
{
    const volatile uint8_t *d = raw[f];
    for (uint32_t k = 0; k < NCLK; k++) {
        uint32_t v = d[k];
#if !DATA_ACTIVE_HIGH
        v = ~v & DATA_MASK;
#endif
        GPIOC->BSRR = v | ((~v & DATA_MASK) << 16) | RST(PIN_CLK);
        GPIOC->BSRR = SET(PIN_CLK);
    }
    GPIOC->BSRR = RST(PIN_CLK);
}

/*
 * TIM3 se desborda cada 500 us. En ese instante el canal 1 (PWM) apaga OE
 * por hardware, y lo vuelve a encender cuando CNT llega a CCR1. Entonces:
 *   [0 .. CCR1)   OE apagado -> aquí se hace latch y se cambia A/B sin "fantasmas"
 *   [CCR1 .. ARR] OE encendido -> la fase se ve
 * Brillo = (ARR+1-CCR1)/(ARR+1).
 *
 * Mientras la fase actual está encendida, ya se desplazan los datos de la
 * siguiente: los registros del panel tienen un latch, así que desplazar no
 * altera lo que se ve hasta el próximo pulso de LAT.
 */
void TIM3_IRQHandler(void)
{
    TIM3->SR = ~TIM_SR_UIF;              /* limpiar bandera de update */

    /* 1) Pasar a las salidas los datos desplazados en el período anterior */
    GPIOC->BSRR = SET(PIN_LAT);
    GPIOC->BSRR = RST(PIN_LAT);

    /* 2) Seleccionar las filas de esa fase con A y B */
    uint8_t f = shifted_phase;
    GPIOC->BSRR = ((f & 1) ? SET(PIN_A) : RST(PIN_A)) |
                  ((f & 2) ? SET(PIN_B) : RST(PIN_B));

    /* 3) Desplazar ya los datos de la fase siguiente */
    f = (f + 1) & (N_PHASES - 1);
    shift_phase(f);
    shifted_phase = f;

    ticks++;
}

/* Brillo en % (1..95). Se limita para que siempre quede un tiempo con OE
   apagado al inicio del período (para latch y cambio de A/B). */
static void set_brightness(uint32_t pct)
{
    if (pct < 1)  pct = 1;
    if (pct > 95) pct = 95;
    uint32_t on_us = PHASE_US * pct / 100u;
    TIM3->CCR1 = PHASE_US - on_us;       /* preload: se aplica en el siguiente update */
}

/* ======================= Configuración FM6126A =======================
 * El FM6126A tiene registros de configuración internos. Se escriben
 * desplazando 16 bits por chip (cada chip maneja 16 columnas) y manteniendo
 * LAT en ALTO durante los últimos N pulsos de CLK:
 *   LAT alto 12 pulsos -> se escribe REG1 (corriente / brillo global)
 *   LAT alto 13 pulsos -> se escribe REG2 (habilita las salidas)
 * Secuencia tomada de librerías abiertas (ESP32-HUB75-MatrixPanel-DMA,
 * SmartMatrix). OE debe estar apagado mientras tanto.
 */
#if USE_FM6126A_INIT
static void delay_short(void) { for (volatile int i = 0; i < 4; i++) { } }

static void fm6126a_write_reg(uint16_t reg, uint32_t lat_clocks)
{
    for (uint32_t k = 0; k < NCLK; k++) {
        uint32_t bit = (reg >> (15 - (k % 16))) & 1u;      /* MSB primero en cada chip */
        uint32_t v = bit ? DATA_MASK : 0;                   /* mismo valor en las 6 líneas */
        uint32_t lat = (k >= NCLK - lat_clocks) ? SET(PIN_LAT) : RST(PIN_LAT);
        GPIOC->BSRR = v | ((~v & DATA_MASK) << 16) | lat | RST(PIN_CLK);
        delay_short();
        GPIOC->BSRR = SET(PIN_CLK);
        delay_short();
    }
    GPIOC->BSRR = RST(PIN_CLK) | RST(PIN_LAT);
    delay_short();
}

static void fm6126a_init(void)
{
    /* REG1: posiciones 5..10 del chip en 1 (bits 10..5) -> corriente de salida */
    fm6126a_write_reg(0x07E0, 12);
    /* REG2: posición 9 del chip en 1 (bit 6) -> habilitar salidas */
    fm6126a_write_reg(0x0040, 13);
}
#endif

/* ======================= Inicialización de periféricos ======================= */
static void gpio_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN;
    (void)RCC->AHB1ENR;   /* lectura de relleno: da tiempo a que el reloj se active */

    /* Estado seguro ANTES de volver salidas los pines: OE apagado, CLK/LAT en 0 */
#if OE_ACTIVE_LOW
    GPIOC->BSRR = SET(PIN_OE);
#else
    GPIOC->BSRR = RST(PIN_OE);
#endif
    GPIOC->BSRR = RST(PIN_CLK) | RST(PIN_LAT) | (DATA_MASK << 16);

    /* PC0-5, PC7, PC8, PC10, PC11: salidas push-pull, velocidad alta */
    const uint32_t outs[] = {PIN_R1, PIN_G1, PIN_B1, PIN_R2, PIN_G2, PIN_B2,
                             PIN_LAT, PIN_CLK, PIN_A, PIN_B};
    for (uint32_t i = 0; i < sizeof outs / sizeof outs[0]; i++) {
        uint32_t p = outs[i];
        GPIOC->MODER   = (GPIOC->MODER   & ~(3u << 2*p)) | (1u << 2*p);  /* 01 = salida */
        GPIOC->OSPEEDR = (GPIOC->OSPEEDR & ~(3u << 2*p)) | (2u << 2*p);  /* 10 = alta   */
    }

    /* PC6: función alterna AF2 = TIM3_CH1 (OE manejado por el PWM) */
    GPIOC->MODER   = (GPIOC->MODER & ~(3u << 2*PIN_OE)) | (2u << 2*PIN_OE);
    GPIOC->OSPEEDR |= (2u << 2*PIN_OE);
    GPIOC->AFR[0]  = (GPIOC->AFR[0] & ~(0xFu << 4*PIN_OE)) | (2u << 4*PIN_OE);

    /* PC13: entrada (botón azul, la Nucleo ya trae pull-up externo) */
    GPIOC->MODER &= ~(3u << 2*13);

    /* PA5: salida (LED verde LD2) */
    GPIOA->MODER = (GPIOA->MODER & ~(3u << 2*5)) | (1u << 2*5);

    /* PA2: AF7 = USART2_TX */
    GPIOA->MODER  = (GPIOA->MODER & ~(3u << 2*2)) | (2u << 2*2);
    GPIOA->AFR[0] = (GPIOA->AFR[0] & ~(0xFu << 4*2)) | (7u << 4*2);
}

static void tim3_init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    (void)RCC->APB1ENR;

    TIM3->PSC = TIM_PSC;               /* 16 MHz / 16 = 1 MHz */
    TIM3->ARR = PHASE_US - 1;          /* 500 cuentas = 500 us */

    /* Canal 1 en PWM modo 1 (OC1M=110) con preload (OC1PE):
       OC1REF = 1 mientras CNT < CCR1, 0 después.
       Con OE activo en bajo y CC1P=0: OE=1 (apagado) al inicio del período,
       OE=0 (encendido) desde CCR1. */
    TIM3->CCMR1 = (6u << 4) | (1u << 3);
#if OE_ACTIVE_LOW
    TIM3->CCER = TIM_CCER_CC1E;                  /* polaridad normal    */
#else
    TIM3->CCER = TIM_CCER_CC1E | TIM_CCER_CC1P;  /* polaridad invertida */
#endif
    set_brightness(25);

    TIM3->CR1 = TIM_CR1_ARPE;          /* ARR con preload */
    TIM3->EGR = TIM_EGR_UG;            /* cargar PSC/ARR/CCR1 ahora (activa UIF) */
    TIM3->SR  = 0;                     /* borrar esa bandera para no entrar de una vez */
    TIM3->DIER = TIM_DIER_UIE;         /* interrupción en cada update */

    NVIC_ISER0 = (1u << TIM3_IRQn);    /* habilitar IRQ 29 en el NVIC */
}

static void uart_init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    (void)RCC->APB1ENR;
    /* BRR = fPCLK1 / baud = 16 000 000 / 115 200 = 138.9 -> 139 (0x8B)
       (mantisa 8, fracción 11/16 con oversampling x16) */
    USART2->BRR = 139;
    USART2->CR1 = USART_CR1_UE | USART_CR1_TE;
}

static void uart_putc(char c)
{
    while (!(USART2->SR & USART_SR_TXE)) { }
    USART2->DR = (uint8_t)c;
}

static void uart_puts(const char *s)
{
    while (*s) uart_putc(*s++);
}

static void uart_putu(uint32_t n)
{
    char buf[11];
    int i = 0;
    do { buf[i++] = (char)('0' + n % 10); n /= 10; } while (n);
    while (i) uart_putc(buf[--i]);
}

/* ======================= Utilidades de la prueba ======================= */
static void clear_raw(void)
{
    for (uint32_t f = 0; f < N_PHASES; f++)
        for (uint32_t k = 0; k < NCLK; k++)
            raw[f][k] = 0;
}

static void fill_phase(uint32_t f, uint8_t color)
{
    for (uint32_t k = 0; k < NCLK; k++)
        raw[f][k] = color;
}

static uint32_t now(void) { return ticks; }

enum { MODE_WALK = 0, MODE_PHASE, MODE_FILL, N_MODES };

static const char *const mode_name[N_MODES] = {
    "WALK (un bit recorre la cadena)",
    "FASE (todas las columnas de una fase)",
    "FILL (pantalla completa R/G/B)",
};

/* ======================= Programa principal ======================= */
int main(void)
{
    gpio_init();
    uart_init();
    clear_raw();

#if USE_FM6126A_INIT
    fm6126a_init();                    /* OE sigue apagado (lo dejó así gpio_init) */
#endif

    /* Precargar los datos de la fase 0 antes de arrancar el timer */
    shifted_phase = 0;
    shift_phase(0);

    tim3_init();
    TIM3->CR1 |= TIM_CR1_CEN;          /* arrancar refresco */

    uart_puts("\r\n=== PRUEBA PANTALLA P10 RGB HUB75 (STM32F446RE) ===\r\n");
    uart_puts("Paneles: "); uart_putu(N_PANELS);
    uart_puts("  CLK por fase: "); uart_putu(NCLK); uart_puts("\r\n");
    uart_puts("Boton azul = cambiar modo\r\n");

    uint32_t mode = MODE_WALK, step = 0;
    uint32_t t_step = now(), t_led = now(), t_btn = now();
    uint32_t btn_prev = 1;
    int redraw = 1;

    while (1) {
        /* ---- Latido: LED verde a 2 Hz (si parpadea, el ISR está vivo) ---- */
        if (now() - t_led >= TICKS_PER_S / 4) {
            t_led = now();
            GPIOA->ODR ^= (1u << 5);
        }

        /* ---- Botón PC13 (activo en bajo) con antirrebote de 50 ms ---- */
        uint32_t btn = (GPIOC->IDR >> 13) & 1u;
        if (btn != btn_prev && now() - t_btn >= TICKS_PER_S / 20) {
            t_btn = now();
            btn_prev = btn;
            if (btn == 0) {                       /* flanco de bajada = presionado */
                mode = (mode + 1) % N_MODES;
                step = 0;
                redraw = 1;
            }
        }

        if (redraw) {
            redraw = 0;
            uart_puts("\r\n>> MODO "); uart_putu(mode);
            uart_puts(": "); uart_puts(mode_name[mode]); uart_puts("\r\n");
            /* Más brillo cuando hay pocos LEDs encendidos; poco en FILL (corriente) */
            set_brightness(mode == MODE_WALK ? 90 : mode == MODE_PHASE ? 40 : 15);
            t_step = now() - 10 * TICKS_PER_S;   /* forzar primer paso ya */
        }

        /* ---- Avance de cada modo ---- */
        if (mode == MODE_WALK) {
            if (now() - t_step >= TICKS_PER_S / 3) {      /* 3 pasos por segundo */
                t_step = now();
                uint32_t f = step / NCLK, k = step % NCLK;
                clear_raw();
                raw[f][k] = C_R1 | C_B2;                    /* rojo arriba, azul abajo */
                uart_puts("fase="); uart_putu(f);
                uart_puts(" (B="); uart_putu((f >> 1) & 1);
                uart_puts(" A=");   uart_putu(f & 1);
                uart_puts(") bit="); uart_putu(k); uart_puts("\r\n");
                step = (step + 1) % (N_PHASES * NCLK);
            }
        } else if (mode == MODE_PHASE) {
            if (now() - t_step >= TICKS_PER_S) {           /* 1 fase por segundo */
                t_step = now();
                uint32_t f = step % N_PHASES;
                clear_raw();
                fill_phase(f, C_R1 | C_G2);                 /* rojo arriba, verde abajo */
                uart_puts("fase="); uart_putu(f); uart_puts("\r\n");
                step++;
            }
        } else { /* MODE_FILL */
            if (now() - t_step >= TICKS_PER_S) {
                t_step = now();
                static const uint8_t colors[3] = { C_R1 | C_R2, C_G1 | C_G2, C_B1 | C_B2 };
                static const char *const cname[3] = { "ROJO", "VERDE", "AZUL" };
                uint32_t c = step % 3;
                for (uint32_t f = 0; f < N_PHASES; f++)
                    fill_phase(f, colors[c]);
                uart_puts(cname[c]); uart_puts("\r\n");
                step++;
            }
        }
    }
}
