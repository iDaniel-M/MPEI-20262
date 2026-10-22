/*
 * serpiente.c - Demo: una serpiente de colores R, G, B recorre todo el panel P10 RGB.
 *
 * - Cada LED del cuerpo tiene un color: cabeza ROJA, luego VERDE, luego AZUL, y se repite.
 * - La serpiente recorre el panel en zig-zag: fila 0 de izquierda a derecha, fila 1 de
 *   derecha a izquierda, etc. (512 posiciones), y vuelve a empezar.
 *
 * Partes:
 *   1) Refresco HUB75 por interrupción de TIM3, igual que main.c (500 us por fase,
 *      OE por PWM de hardware en PC6 = TIM3_CH1).
 *   2) set_pixel(x, y, color): usa el mapeo medido en MAPEO.md para pasar de (x,y) a
 *      (fase, bit) del buffer crudo.
 *   3) Doble buffer: se dibuja en un buffer mientras la ISR muestra el otro; se
 *      intercambian al comenzar un cuadro (fase 0), así nunca se ve un dibujo a medias.
 *
 * Compilar y grabar:  make serpiente && make flash-serpiente
 */
#include <stdint.h>
#include "regs.h"

/* ======================= Parámetros ======================= */
#define NCLK      64u      /* pulsos de CLK por fase (1 panel)  */
#define N_PHASES  4u       /* escaneo 1/4                        */
#define W         32u      /* ancho en píxeles                   */
#define H         16u      /* alto en píxeles                    */
#define TIM_PSC   15u      /* 16 MHz / 16 = 1 MHz -> 1 tick = 1 us */
#define PHASE_US  500u     /* 500 us por fase -> 500 cuadros/s   */
#define TICKS_PER_S 2000u  /* interrupciones por segundo         */

#define SNAKE_LEN   24u    /* LEDs del cuerpo (múltiplo de 3 = R,G,B completos) */
#define STEP_TICKS  80u    /* 80 x 0.5 ms = 40 ms por paso -> 25 pasos/s        */
#define BRIGHT_PCT  36u    /* brillo en % del tiempo con OE encendido. Antes 60; se bajó al 60 % de eso (36) porque deslumbraba */

/* Pines (GPIOC), los mismos de main.c */
#define PIN_OE  6
#define PIN_LAT 7
#define PIN_CLK 8
#define PIN_A   10
#define PIN_B   11
#define SET(p) (1u << (p))
#define RST(p) (1u << ((p) + 16))
#define DATA_MASK 0x3Fu    /* PC0..PC5 = R1 G1 B1 R2 G2 B2 */

/* Colores: bit0 = R, bit1 = G, bit2 = B (se desplazan 3 bits para la mitad inferior) */
enum { RED = 1, GREEN = 2, BLUE = 4 };

/* ======================= Doble buffer =======================
 * buf[n][f][k] = valor de las 6 líneas de dato en el pulso k de la fase f. */
static uint8_t buf[2][N_PHASES][NCLK];
static volatile uint8_t front = 0;        /* buffer que muestra la ISR */
static volatile uint8_t swap_req = 0;     /* 1 = el main terminó de dibujar en el otro */
static volatile uint32_t ticks;
static uint8_t shifted_phase;

/* ======================= Mapeo (x,y) -> buffer (ver MAPEO.md) =======================
 * En cada fase hay 4 bloques de 16 bits (uno cada 8 columnas):
 *   filas 0-3 de cada mitad: primeros 8 bits del bloque, de derecha a izquierda
 *   filas 4-7 de cada mitad: últimos 8 bits del bloque, de izquierda a derecha
 * La mitad inferior (y >= 8) usa las líneas R2/G2/B2 = mismos bits desplazados 3. */
static void set_pixel(uint8_t (*b)[NCLK], uint32_t x, uint32_t y, uint32_t color)
{
    uint32_t half = y / 8;
    uint32_t yy   = y % 8;
    uint32_t f    = yy % 4;
    uint32_t j    = (yy < 4) ? 7 - (x % 8) : 8 + (x % 8);
    uint32_t k    = 16 * (x / 8) + j;
    b[f][k] |= (uint8_t)(color << (3 * half));
}

/* ======================= Refresco HUB75 (igual que main.c) ======================= */
static void shift_phase(uint8_t f)
{
    const uint8_t *d = buf[front][f];
    for (uint32_t k = 0; k < NCLK; k++) {
        uint32_t v = d[k];
        GPIOC->BSRR = v | ((~v & DATA_MASK) << 16) | RST(PIN_CLK);  /* dato + CLK abajo */
        GPIOC->BSRR = SET(PIN_CLK);                                 /* flanco de subida */
    }
    GPIOC->BSRR = RST(PIN_CLK);
}

/* TIM3 se desborda cada 500 us: al inicio del período OE está apagado por el PWM,
 * se hace latch de lo desplazado antes, se cambian A/B y se desplaza la fase siguiente. */
void TIM3_IRQHandler(void)
{
    TIM3->SR = ~TIM_SR_UIF;

    GPIOC->BSRR = SET(PIN_LAT);                    /* 1) latch */
    GPIOC->BSRR = RST(PIN_LAT);

    uint8_t f = shifted_phase;                     /* 2) filas de esa fase */
    GPIOC->BSRR = ((f & 1) ? SET(PIN_A) : RST(PIN_A)) |
                  ((f & 2) ? SET(PIN_B) : RST(PIN_B));

    f = (f + 1) & (N_PHASES - 1);                  /* 3) siguiente fase */
    if (f == 0 && swap_req) {                      /* cuadro nuevo: cambiar de buffer */
        front ^= 1;
        swap_req = 0;
    }
    shift_phase(f);
    shifted_phase = f;
    ticks++;
}

/* ======================= Inicialización (igual que main.c) ======================= */
static void gpio_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN;
    (void)RCC->AHB1ENR;

    GPIOC->BSRR = SET(PIN_OE) | RST(PIN_CLK) | RST(PIN_LAT) | (DATA_MASK << 16);

    const uint32_t outs[] = {0, 1, 2, 3, 4, 5, PIN_LAT, PIN_CLK, PIN_A, PIN_B};
    for (uint32_t i = 0; i < sizeof outs / sizeof outs[0]; i++) {
        uint32_t p = outs[i];
        GPIOC->MODER   = (GPIOC->MODER   & ~(3u << 2*p)) | (1u << 2*p);  /* salida */
        GPIOC->OSPEEDR = (GPIOC->OSPEEDR & ~(3u << 2*p)) | (2u << 2*p);  /* alta velocidad */
    }
    /* PC6 = AF2 (TIM3_CH1): OE por PWM */
    GPIOC->MODER  = (GPIOC->MODER & ~(3u << 2*PIN_OE)) | (2u << 2*PIN_OE);
    GPIOC->OSPEEDR |= (2u << 2*PIN_OE);
    GPIOC->AFR[0] = (GPIOC->AFR[0] & ~(0xFu << 4*PIN_OE)) | (2u << 4*PIN_OE);

    GPIOA->MODER = (GPIOA->MODER & ~(3u << 2*5)) | (1u << 2*5);          /* PA5 = LD2 */
}

static void tim3_init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    (void)RCC->APB1ENR;
    TIM3->PSC   = TIM_PSC;
    TIM3->ARR   = PHASE_US - 1;
    TIM3->CCMR1 = (6u << 4) | (1u << 3);           /* PWM modo 1 + preload en CH1 */
    TIM3->CCER  = TIM_CCER_CC1E;                   /* OE activo en bajo: polaridad normal */
    TIM3->CCR1  = PHASE_US - PHASE_US * BRIGHT_PCT / 100u;  /* OE encendido desde CCR1 */
    TIM3->CR1   = TIM_CR1_ARPE;
    TIM3->EGR   = TIM_EGR_UG;
    TIM3->SR    = 0;
    TIM3->DIER  = TIM_DIER_UIE;
    NVIC_ISER0  = (1u << TIM3_IRQn);
}

/* ======================= Recorrido en zig-zag =======================
 * Posición p (0..511) -> (x,y): filas pares de izquierda a derecha, impares al revés,
 * así la serpiente baja de fila sin "saltar" de un borde al otro. */
static void path_xy(uint32_t p, uint32_t *x, uint32_t *y)
{
    *y = p / W;
    *x = (*y % 2 == 0) ? p % W : (W - 1) - p % W;
}

int main(void)
{
    gpio_init();
    shifted_phase = 0;
    shift_phase(0);
    tim3_init();
    TIM3->CR1 |= TIM_CR1_CEN;

    static const uint8_t colors[3] = { RED, GREEN, BLUE };
    uint32_t head = 0, t_step = ticks;

    while (1) {
        if (ticks - t_step < STEP_TICKS || swap_req) continue;   /* esperar el próximo paso */
        t_step = ticks;

        /* Dibujar en el buffer que NO se está mostrando */
        uint8_t (*b)[NCLK] = buf[front ^ 1];
        for (uint32_t f = 0; f < N_PHASES; f++)
            for (uint32_t k = 0; k < NCLK; k++)
                b[f][k] = 0;

        for (uint32_t i = 0; i < SNAKE_LEN; i++) {
            /* segmento i está i posiciones detrás de la cabeza (con vuelta al final) */
            uint32_t p = (head + W * H - i) % (W * H), x, y;
            path_xy(p, &x, &y);
            set_pixel(b, x, y, colors[i % 3]);    /* R, G, B, R, G, B, ... */
        }
        /* Barrera de compilador: buf no es volatile, así que sin esto GCC podría
           mover escrituras del dibujo DESPUÉS de swap_req = 1 y la ISR vería un
           dibujo a medias. "memory" le prohíbe reordenar accesos a memoria aquí. */
        __asm volatile ("" ::: "memory");
        swap_req = 1;                             /* la ISR lo mostrará en el próximo cuadro */

        head = (head + 1) % (W * H);
        GPIOA->ODR ^= (1u << 5);                  /* LD2 parpadea con cada paso */
    }
}
