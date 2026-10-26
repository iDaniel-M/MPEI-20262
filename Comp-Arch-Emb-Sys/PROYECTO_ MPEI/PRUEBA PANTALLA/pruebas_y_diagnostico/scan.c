/*
 * scan.c - BARRIDO AUTOMÁTICO DE TIPOS DE DRIVER para el panel P10 RGB HUB75
 *
 * Si el panel no enciende con el programa normal, este firmware prueba solo,
 * uno tras otro, las configuraciones más comunes de los chips driver:
 *
 *   inicialización:  ninguna (ICN2037, MBI5024, SM16126, 74HC595...)
 *                    FM6126A / ICN2038S  (REG1=0x07E0, REG2=0x0040)
 *                    FM6126A variante    (REG1=0x7FFF, REG2=0x0040)
 *   OE:              activo en bajo / activo en alto
 *   reloj (CLK):     dato capturado en flanco de subida / de bajada
 *
 *   3 x 2 x 2 = 12 configuraciones, 5 s cada una (1 minuto por vuelta).
 *
 * Patrón mostrado: la MITAD de la cadena encendida (rojo arriba, verde abajo).
 * Así se ve algo aunque los datos estuvieran invertidos.
 *
 * Cómo saber cuál funcionó:
 *   - El LED verde LD2 da N destellos cortos y una pausa: N = número de config.
 *   - Por UART (115200) se imprime el nombre de cada configuración.
 *   - Cuando la pantalla encienda, presionar el BOTÓN AZUL: se queda fija en
 *     esa configuración (LD2 sigue mostrando el número). Otra vez = continuar.
 *
 * Pines: los mismos de main.c (R1..B2=PC0..PC5, OE=PC6/TIM3_CH1, LAT=PC7,
 * CLK=PC8, A=PC10, B=PC11).
 */
#include <stdint.h>
#include "regs.h"

#ifndef N_PANELS
#define N_PANELS 1
#endif
#define BITS_PER_PANEL 64
#define NCLK (BITS_PER_PANEL * N_PANELS)
#define N_PHASES 4

#define TIM_PSC     15u          /* 16 MHz / 16 = 1 MHz */
#define PHASE_US    500u
#define TICKS_PER_S 2000u
#define CONFIG_SECONDS 5u
#define BRIGHT_PCT  30u

#define PIN_OE  6
#define PIN_LAT 7
#define PIN_CLK 8
#define PIN_A   10
#define PIN_B   11
#define SET(p) (1u << (p))
#define RST(p) (1u << ((p) + 16))
#define DATA_MASK 0x3Fu
#define C_R1 (1u << 0)
#define C_G2 (1u << 4)

/* ---------------- Tabla de configuraciones ---------------- */
typedef struct {
    const char *name;
    uint8_t  init;      /* 0 = ninguna, 1 = escribir REG1/REG2 estilo FM6126A */
    uint16_t reg1, reg2;
    uint8_t  oe_low;    /* 1 = OE activo en bajo */
    uint8_t  clk_fall;  /* 1 = dato capturado en flanco de bajada */
} cfg_t;

static const cfg_t cfgs[] = {
    {"sin init           OE bajo  CLK subida", 0, 0,      0,      1, 0},
    {"sin init           OE alto  CLK subida", 0, 0,      0,      0, 0},
    {"sin init           OE bajo  CLK bajada", 0, 0,      0,      1, 1},
    {"sin init           OE alto  CLK bajada", 0, 0,      0,      0, 1},
    {"FM6126A 07E0/0040  OE bajo  CLK subida", 1, 0x07E0, 0x0040, 1, 0},
    {"FM6126A 07E0/0040  OE alto  CLK subida", 1, 0x07E0, 0x0040, 0, 0},
    {"FM6126A 07E0/0040  OE bajo  CLK bajada", 1, 0x07E0, 0x0040, 1, 1},
    {"FM6126A 07E0/0040  OE alto  CLK bajada", 1, 0x07E0, 0x0040, 0, 1},
    {"FM6126A 7FFF/0040  OE bajo  CLK subida", 1, 0x7FFF, 0x0040, 1, 0},
    {"FM6126A 7FFF/0040  OE alto  CLK subida", 1, 0x7FFF, 0x0040, 0, 0},
    {"FM6126A 7FFF/0040  OE bajo  CLK bajada", 1, 0x7FFF, 0x0040, 1, 1},
    {"FM6126A 7FFF/0040  OE alto  CLK bajada", 1, 0x7FFF, 0x0040, 0, 1},
};
#define N_CFGS (sizeof cfgs / sizeof cfgs[0])

/* ---------------- Estado ---------------- */
static volatile uint8_t  raw[N_PHASES][NCLK];
static volatile uint32_t ticks;
static volatile uint8_t  shifted_phase;

/* Bits de BSRR para CLK: "with_data" se escribe junto con el dato,
   "edge" produce el flanco que captura. Cambian según cfg.clk_fall. */
static volatile uint32_t clk_with_data = RST(PIN_CLK);
static volatile uint32_t clk_edge      = SET(PIN_CLK);

/* ---------------- Desplazamiento ---------------- */
static void shift_phase(uint8_t f)
{
    const volatile uint8_t *d = raw[f];
    uint32_t a = clk_with_data, e = clk_edge;
    for (uint32_t k = 0; k < NCLK; k++) {
        uint32_t v = d[k];
        GPIOC->BSRR = v | ((~v & DATA_MASK) << 16) | a;
        GPIOC->BSRR = e;
    }
    GPIOC->BSRR = a;      /* CLK vuelve al nivel de reposo */
}

void TIM3_IRQHandler(void)
{
    TIM3->SR = ~TIM_SR_UIF;
    GPIOC->BSRR = SET(PIN_LAT);
    GPIOC->BSRR = RST(PIN_LAT);
    uint8_t f = shifted_phase;
    GPIOC->BSRR = ((f & 1) ? SET(PIN_A) : RST(PIN_A)) |
                  ((f & 2) ? SET(PIN_B) : RST(PIN_B));
    f = (f + 1) & (N_PHASES - 1);
    shift_phase(f);
    shifted_phase = f;
    ticks++;
}

/* ---------------- Registros tipo FM6126A ---------------- */
static void delay_short(void) { for (volatile int i = 0; i < 4; i++) { } }

static void write_reg(uint16_t reg, uint32_t lat_clocks)
{
    for (uint32_t k = 0; k < NCLK; k++) {
        uint32_t bit = (reg >> (15 - (k % 16))) & 1u;
        uint32_t v = bit ? DATA_MASK : 0;
        uint32_t lat = (k >= NCLK - lat_clocks) ? SET(PIN_LAT) : RST(PIN_LAT);
        GPIOC->BSRR = v | ((~v & DATA_MASK) << 16) | lat | clk_with_data;
        delay_short();
        GPIOC->BSRR = clk_edge;
        delay_short();
    }
    GPIOC->BSRR = clk_with_data | RST(PIN_LAT);
    delay_short();
}

/* ---------------- UART ---------------- */
static void uart_putc(char c) { while (!(USART2->SR & USART_SR_TXE)) { } USART2->DR = (uint8_t)c; }
static void uart_puts(const char *s) { while (*s) uart_putc(*s++); }
static void uart_putu(uint32_t n)
{
    char b[11]; int i = 0;
    do { b[i++] = (char)('0' + n % 10); n /= 10; } while (n);
    while (i) uart_putc(b[--i]);
}

/* ---------------- Periféricos ---------------- */
static void pin_mode(GPIO_t *g, uint32_t p, uint32_t mode)
{
    g->MODER = (g->MODER & ~(3u << 2*p)) | (mode << 2*p);
}

static void hw_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN | RCC_APB1ENR_USART2EN;
    (void)RCC->APB1ENR;

    GPIOC->BSRR = SET(PIN_OE) | RST(PIN_CLK) | RST(PIN_LAT) | (DATA_MASK << 16);
    const uint32_t outs[] = {0, 1, 2, 3, 4, 5, PIN_OE, PIN_LAT, PIN_CLK, PIN_A, PIN_B};
    for (uint32_t i = 0; i < sizeof outs / sizeof outs[0]; i++) {
        pin_mode(GPIOC, outs[i], 1);
        GPIOC->OSPEEDR |= (2u << 2*outs[i]);
    }
    GPIOC->AFR[0] = (GPIOC->AFR[0] & ~(0xFu << 4*PIN_OE)) | (2u << 4*PIN_OE); /* AF2 TIM3_CH1 */
    pin_mode(GPIOC, 13, 0);                     /* botón */
    pin_mode(GPIOA, 5, 1);                      /* LD2 */
    pin_mode(GPIOA, 2, 2);                      /* PA2 USART2_TX */
    GPIOA->AFR[0] = (GPIOA->AFR[0] & ~(0xFu << 8)) | (7u << 8);

    USART2->BRR = 139;                          /* 16 MHz / 115200 */
    USART2->CR1 = USART_CR1_UE | USART_CR1_TE;

    TIM3->PSC  = TIM_PSC;
    TIM3->ARR  = PHASE_US - 1;
    TIM3->CCR1 = PHASE_US - PHASE_US * BRIGHT_PCT / 100u;
    TIM3->CCMR1 = (6u << 4) | (1u << 3);        /* PWM modo 1 + preload */
    TIM3->CR1  = TIM_CR1_ARPE;
    NVIC_ISER0 = (1u << TIM3_IRQn);
}

/* Detener refresco, aplicar la configuración c y volver a arrancar */
static void apply_cfg(const cfg_t *c)
{
    /* 1) parar timer e interrupción */
    TIM3->CR1 &= ~TIM_CR1_CEN;
    TIM3->DIER = 0;
    TIM3->SR = 0;

    /* 2) OE como GPIO en su nivel "apagado" mientras se configura */
    GPIOC->BSRR = c->oe_low ? SET(PIN_OE) : RST(PIN_OE);
    pin_mode(GPIOC, PIN_OE, 1);

    /* 3) flanco de reloj */
    if (c->clk_fall) { clk_with_data = SET(PIN_CLK); clk_edge = RST(PIN_CLK); }
    else             { clk_with_data = RST(PIN_CLK); clk_edge = SET(PIN_CLK); }
    GPIOC->BSRR = clk_with_data;

    /* 4) registros del driver */
    if (c->init) {
        write_reg(c->reg1, 12);
        write_reg(c->reg2, 13);
    }

    /* 5) patrón: mitad de la cadena encendida en todas las fases */
    for (uint32_t f = 0; f < N_PHASES; f++)
        for (uint32_t k = 0; k < NCLK; k++)
            raw[f][k] = (k < NCLK / 2) ? (C_R1 | C_G2) : 0;
    shifted_phase = 0;
    shift_phase(0);

    /* 6) polaridad de OE en el PWM y volver a darle el pin al timer */
    TIM3->CCER = c->oe_low ? TIM_CCER_CC1E : (TIM_CCER_CC1E | TIM_CCER_CC1P);
    TIM3->CNT = 0;
    TIM3->EGR = TIM_EGR_UG;
    TIM3->SR  = 0;
    pin_mode(GPIOC, PIN_OE, 2);
    TIM3->DIER = TIM_DIER_UIE;
    TIM3->CR1 |= TIM_CR1_CEN;
}

/* LD2: N destellos de 150 ms y una pausa de 900 ms, repetido */
static void led_show(uint32_t n, uint32_t t)
{
    uint32_t slot = (t / (TICKS_PER_S * 15 / 100)) % (2 * n + 6);
    if (slot < 2 * n && (slot & 1) == 0) GPIOA->BSRR = SET(5);
    else                                 GPIOA->BSRR = RST(5);
}

int main(void)
{
    hw_init();
    uart_puts("\r\n=== BARRIDO DE DRIVERS P10 RGB ===\r\n");
    uart_puts("Boton azul: fijar / continuar\r\n");

    uint32_t idx = 0, locked = 0;
    uint32_t t_cfg, t_btn = 0, btn_prev = 1;

    apply_cfg(&cfgs[idx]);
    t_cfg = ticks;
    uart_puts("Config "); uart_putu(idx + 1); uart_puts(": "); uart_puts(cfgs[idx].name); uart_puts("\r\n");

    while (1) {
        uint32_t t = ticks;
        led_show(idx + 1, t);

        /* botón con antirrebote */
        uint32_t btn = (GPIOC->IDR >> 13) & 1u;
        if (btn != btn_prev && t - t_btn >= TICKS_PER_S / 20) {
            t_btn = t; btn_prev = btn;
            if (btn == 0) {
                locked = !locked;
                uart_puts(locked ? ">> FIJADA config " : ">> continuar desde config ");
                uart_putu(idx + 1); uart_puts("\r\n");
                t_cfg = t;
            }
        }

        if (!locked && t - t_cfg >= CONFIG_SECONDS * TICKS_PER_S) {
            idx = (idx + 1) % N_CFGS;
            apply_cfg(&cfgs[idx]);
            t_cfg = ticks;
            uart_puts("Config "); uart_putu(idx + 1); uart_puts(": "); uart_puts(cfgs[idx].name); uart_puts("\r\n");
        }
    }
}
