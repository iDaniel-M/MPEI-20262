/*
 * pruebas.c - Banco de pruebas del protocolo HUB75 (una variante por compilación)
 *
 * Problema: el panel muestra ~4-5 LEDs azules fijos (basura del arranque) y NO
 * cambia con los datos que enviamos. Si CLK y LAT llegaran bien al panel, esa
 * basura se sobrescribiría (aunque los datos R/G/B no llegaran, se cargarían
 * ceros y se apagaría). Por eso cada variante cambia UNA cosa del protocolo y
 * miramos si la basura cambia.
 *
 * Todas las variantes intentan mostrar la pantalla COMPLETA EN ROJO (R1 y R2 = 1).
 * Cualquier cambio respecto a los 4 LEDs azules = algo entró al panel.
 *
 * Se compila con:   make pruebas VAR=n   y se graba con   make flash-pruebas
 *
 * A diferencia de main.c, aquí NO se usa TIM3: todo se hace por GPIO desde el
 * lazo principal ("bit-banging"), con retardos exactos medidos con el contador
 * de ciclos DWT_CYCCNT. Así el tiempo de cada señal es fácil de controlar.
 *
 * Al arrancar también se hace un AUTODIAGNÓSTICO de pines (ver pin_diag()) que
 * se imprime por la UART (115200 8N1, Virtual COM del ST-LINK).
 */
#include <stdint.h>
#include "regs.h"

#ifndef VAR
#define VAR 0
#endif

/* MIRROR (se pasa desde make: MIRROR=1) - HIPÓTESIS del autodiagnóstico:
   PC2, PC5 y PC6 están en corto a GND. Son los pines HUB75 3, 7 y 15, y sus
   vecinos de la otra fila (4, 8 y 16) son GND. Si el conector está ESPEJADO
   (pin 1<->2, 3<->4, ..., 15<->16), cada cable llega al pin vecino:
     PC0 -> G1   PC1 -> R1   PC2 -> GND(4)   PC3 -> G2   PC4 -> R2   PC5 -> GND(8)
     PC10 -> B   PC11 -> A   PC7 -> CLK      PC8 -> LAT  PC6 -> GND(16)
     GND de la Nucleo -> OE del panel (OE siempre activo = "siempre encendido")
   Con MIRROR=1 las señales se reasignan por software para comprobarlo. */
#ifndef MIRROR
#define MIRROR 0
#endif

/* Historia: el 22-oct PC2 (B1), PC5 (B2) y PC6 (OE) leían 0 aun manejados en
   ALTO (corto a GND): las dos filas del conector HUB75 estaban invertidas y esos
   cables caían en los GND 4, 8 y 16. Manejar en alto un pin en corto lo fuerza
   (decenas de mA), por eso con 1 esos tres pines quedan como ENTRADAS.
   Cableado corregido y autodiagnóstico sin cortos -> 0 (se usan los 11 pines). */
#define PINES_EN_CORTO 0

/* ===================== Parámetros por defecto =====================
 * Cada variante (más abajo) sobrescribe solo lo que quiere probar. */
#define CLK_HALF_US    0   /* medio período de CLK en us (0 = lo más rápido posible, ~8 MHz/2) */
#define LAT_US         1   /* duración del pulso de LAT en us */
#define LAT_ACT_LOW    0   /* 1 = LAT en reposo alto y pulso bajo */
#define DATA_INV       0   /* 1 = invertir las 6 líneas de color */
#define NBITS          64  /* pulsos de CLK por fase (32 col x 2 filas en 1/4 scan) */
#define OE_MODE        0   /* 0 = OE apagado al desplazar y encendido al mostrar
                              1 = OE SIEMPRE en 1 (apagado si es activo en bajo)
                              2 = OE SIEMPRE en 0 (encendido si es activo en bajo) */
#define CLK_FALLING    0   /* 1 = el dato se pone con CLK en alto y se captura en el flanco de BAJADA */
#define OPEN_DRAIN     0   /* 1 = salidas en drenador abierto (alto = pin liberado) */
#define FM6126A        0   /* 1 = enviar configuración FM6126A antes de mostrar */
#define PWM_DRIVER     0   /* 1 = intento de protocolo ICN2053/FM6353 (GCLK + VSYNC) */
#define DESC "base: CLK rapido, LAT 1us, 64 bits, OE por GPIO"

/* ===================== Variantes ===================== */
#if VAR == 0
/* Igual que main.c pero sin timer: verifica que el bit-banging básico se comporta igual. */
#elif VAR == 1
#undef OE_MODE
#define OE_MODE 1
#undef DESC
#define DESC "OE fijo en 1 (deberia APAGAR todo si OE es activo en bajo)"
#elif VAR == 2
#undef OE_MODE
#define OE_MODE 2
#undef DESC
#define DESC "OE fijo en 0 (todo encendido segun lo que haya en el latch)"
#elif VAR == 3
#undef CLK_HALF_US
#define CLK_HALF_US 1
#undef DESC
#define DESC "CLK lento: 1 us en alto + 1 us en bajo (500 kHz)"
#elif VAR == 4
#undef CLK_HALF_US
#define CLK_HALF_US 1
#undef LAT_US
#define LAT_US 20
#undef DESC
#define DESC "CLK lento + LAT largo (20 us)"
#elif VAR == 5
#undef CLK_HALF_US
#define CLK_HALF_US 1
#undef LAT_ACT_LOW
#define LAT_ACT_LOW 1
#undef LAT_US
#define LAT_US 5
#undef DESC
#define DESC "LAT activo en BAJO (reposo alto), CLK lento"
#elif VAR == 6
#undef CLK_HALF_US
#define CLK_HALF_US 1
#undef DATA_INV
#define DATA_INV 1
#undef DESC
#define DESC "Datos invertidos (rojo -> se envian G y B en 1), CLK lento"
#elif VAR == 7
#undef CLK_HALF_US
#define CLK_HALF_US 1
#undef NBITS
#define NBITS 32
#undef DESC
#define DESC "32 bits por fase, CLK lento"
#elif VAR == 8
#undef CLK_HALF_US
#define CLK_HALF_US 1
#undef NBITS
#define NBITS 128
#undef DESC
#define DESC "128 bits por fase, CLK lento"
#elif VAR == 9
#undef CLK_HALF_US
#define CLK_HALF_US 1
#undef NBITS
#define NBITS 256
#undef DESC
#define DESC "256 bits por fase, CLK lento"
#elif VAR == 10
#undef CLK_HALF_US
#define CLK_HALF_US 1
#undef FM6126A
#define FM6126A 1
#undef DESC
#define DESC "Init FM6126A (REG1=7FFF, REG2=0040) + CLK lento"
#elif VAR == 11
#undef CLK_HALF_US
#define CLK_HALF_US 1
#undef CLK_FALLING
#define CLK_FALLING 1
#undef DESC
#define DESC "Captura en flanco de BAJADA de CLK, CLK lento"
#elif VAR == 12
#undef CLK_HALF_US
#define CLK_HALF_US 2
#undef LAT_US
#define LAT_US 10
#undef OPEN_DRAIN
#define OPEN_DRAIN 1
#undef DESC
#define DESC "Drenador abierto (alto lo da el pull-up del panel, si lo tiene)"
#elif VAR == 13
#undef CLK_HALF_US
#define CLK_HALF_US 1
#undef PWM_DRIVER
#define PWM_DRIVER 1
#undef DESC
#define DESC "Intento ICN2053/FM6353: config + datos 16 bits + VSYNC + GCLK continuo"
#elif VAR == 14
#undef CLK_HALF_US
#define CLK_HALF_US 50
#undef LAT_US
#define LAT_US 100
#undef DESC
#define DESC "CLK MUY lento (50 us + 50 us = 10 kHz), LAT 100 us"
#elif VAR == 16
/* CONTROL POSITIVO de A/B: no se manda CLK/LAT/datos, solo se fijan A y B en el
   valor ROW (make ... ROW=0..3). Si los LEDs de basura cambian de fila entre
   ROW=0,1,2,3, el panel SÍ recibe A y B, incluidos los niveles ALTOS de 3.3 V. */
#ifndef ROW
#define ROW 0
#endif
#undef DESC
#define DESC "Solo A/B fijos (control positivo de filas)"
#elif VAR == 17
/* "FUZZ": no supone qué pin es CLK, LAT o dato. Cada 1 us se escribe un valor
   pseudoaleatorio (LFSR de 16 bits) en PC0, PC1, PC3, PC4, PC7 y PC8 a la vez.
   Si CLK y LAT del panel están en CUALQUIERA de esos pines, habrá flancos de CLK
   con datos variables y pulsos de LAT -> el contenido del latch tiene que cambiar.
   Si la basura sigue igual, CLK/LAT no llegan por ninguno de esos pines. */
#undef DESC
#define DESC "FUZZ: LFSR en PC0 PC1 PC3 PC4 PC7 PC8, A/B barriendo"
#elif VAR == 18
/* MAPA: mismo patrón que el modo WALK de main.c (un solo bit encendido: R1 rojo
   arriba y B2 azul abajo), pero en vez de avanzar 3 veces por segundo se queda
   4 s en cada (fase, bit) de una lista elegida. Así cada foto de la webcam se
   puede asociar sin ambigüedad con la línea "fase= bit=" de la UART. */
#undef DESC
#define DESC "MAPA: WALK lento en (fase,bit) elegidos, 4 s cada uno"
#elif VAR == 19
/* VALIDACIÓN DEL MAPEO (MAPEO.md): se dibuja una imagen conocida usando la
   fórmula inversa (x,y) -> (fase,bit). Si la fórmula es correcta se ve:
   marco VERDE en todo el borde, diagonal ROJA de (0,0) a (15,15) y diagonal
   AZUL de (16,0) a (31,15). Cualquier error de la fórmula rompe las figuras. */
#undef DESC
#define DESC "VALIDA: marco verde + diagonal roja + diagonal azul con el mapeo"
#else
#error "VAR no definida"
#endif

/* ===================== Pines (GPIOC) ===================== */
#define PIN_OE  6
#define SET(p) (1u << (p))           /* BSRR bits 0-15: poner en 1 */
#define RST(p) (1u << ((p) + 16))    /* BSRR bits 16-31: poner en 0 */
#if MIRROR
/* Cable espejado: cada señal sale por el pin del cable vecino (ver MIRROR arriba). */
#define PIN_LAT 8
#define PIN_CLK 7
#define PIN_A   11
#define PIN_B   10
#define RED  ((1u << 1) | (1u << 4)) /* PC1 -> R1, PC4 -> R2 */
#else
#define PIN_LAT 7
#define PIN_CLK 8
#define PIN_A   10
#define PIN_B   11
#define RED  ((1u << 0) | (1u << 3)) /* R1 y R2 */
#endif
#if PINES_EN_CORTO
#define DATA_MASK ((1u << 0) | (1u << 1) | (1u << 3) | (1u << 4))  /* PC0 PC1 PC3 PC4 (sin PC2/PC5) */
#else
#define DATA_MASK 0x3Fu              /* PC0..PC5 = R1 G1 B1 R2 G2 B2 */
#endif

/* ===================== Retardos con DWT =====================
 * DWT (Data Watchpoint and Trace) tiene un contador de ciclos de CPU de 32 bits.
 * A 16 MHz (HSI), 1 us = 16 ciclos. Hay que habilitar primero el bloque de
 * trazas con DEMCR.TRCENA (bit 24) y luego DWT_CTRL.CYCCNTENA (bit 0). */
#define CPU_MHZ 16u
static void dwt_init(void)
{
    DEMCR     |= (1u << 24);   /* TRCENA: habilita DWT/ITM */
    DWT_CYCCNT = 0;
    DWT_CTRL  |= 1u;           /* CYCCNTENA: el contador empieza a contar */
}
static void delay_us(uint32_t us)
{
    uint32_t t0 = DWT_CYCCNT;               /* la resta sin signo funciona aunque desborde */
    while (DWT_CYCCNT - t0 < us * CPU_MHZ) { }
}
#if CLK_HALF_US
#define CLK_WAIT() delay_us(CLK_HALF_US)
#else
#define CLK_WAIT() ((void)0)
#endif

/* ===================== UART (solo TX, para el diagnóstico) ===================== */
static void uart_init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    (void)RCC->APB1ENR;
    USART2->BRR = 139;                       /* 16 MHz / 115200 = 138.9 */
    USART2->CR1 = USART_CR1_UE | USART_CR1_TE;
}
static void uart_putc(char c) { while (!(USART2->SR & USART_SR_TXE)) { } USART2->DR = (uint8_t)c; }
static void uart_puts(const char *s) { while (*s) uart_putc(*s++); }
static void uart_putu(uint32_t n)
{
    char b[11]; int i = 0;
    do { b[i++] = (char)('0' + n % 10); n /= 10; } while (n);
    while (i) uart_putc(b[--i]);
}

/* ===================== Señales básicas ===================== */
static inline void oe_set(int on)      /* on=1 -> LEDs encendidos (OE activo en bajo) */
{
#if MIRROR || PINES_EN_CORTO
    (void)on;                           /* PC6 en corto / OE del panel en GND: no se controla */
#elif OE_MODE == 1
    (void)on; GPIOC->BSRR = SET(PIN_OE);
#elif OE_MODE == 2
    (void)on; GPIOC->BSRR = RST(PIN_OE);
#else
    GPIOC->BSRR = on ? RST(PIN_OE) : SET(PIN_OE);
#endif
}

static inline void lat_level(int active)
{
#if LAT_ACT_LOW
    GPIOC->BSRR = active ? RST(PIN_LAT) : SET(PIN_LAT);
#else
    GPIOC->BSRR = active ? SET(PIN_LAT) : RST(PIN_LAT);
#endif
}

/* Un pulso de CLK con el valor v en las 6 líneas de color.
 * Flanco de subida (normal): CLK=0 + dato -> espera -> CLK=1 (captura) -> espera.
 * Flanco de bajada: CLK=1 + dato -> espera -> CLK=0 (captura) -> espera. */
static inline void clk_bit(uint32_t v)
{
#if DATA_INV
    v = ~v;
#endif
    v &= DATA_MASK;
    uint32_t data = v | ((~v & DATA_MASK) << 16);   /* 1s con SET, 0s con RST en una sola escritura */
#if CLK_FALLING
    GPIOC->BSRR = data | SET(PIN_CLK);
    CLK_WAIT();
    GPIOC->BSRR = RST(PIN_CLK);
    CLK_WAIT();
#else
    GPIOC->BSRR = data | RST(PIN_CLK);
    CLK_WAIT();
    GPIOC->BSRR = SET(PIN_CLK);
    CLK_WAIT();
#endif
}

__attribute__((unused))
static void latch_pulse(void)
{
    lat_level(1);
    delay_us(LAT_US);
    lat_level(0);
    delay_us(1);
}

static void set_rows(uint32_t f)
{
    GPIOC->BSRR = ((f & 1) ? SET(PIN_A) : RST(PIN_A)) |
                  ((f & 2) ? SET(PIN_B) : RST(PIN_B));
}

/* ===================== GPIO ===================== */
#if MIRROR || PINES_EN_CORTO
static const uint8_t out_pins[] = {0, 1, 3, 4, 7, 8, 10, 11};   /* PC2, PC5, PC6 quedan como entradas */
#else
static const uint8_t out_pins[] = {0, 1, 2, 3, 4, 5, PIN_OE, PIN_LAT, PIN_CLK, PIN_A, PIN_B};
#endif
#define N_OUT (sizeof out_pins / sizeof out_pins[0])

static void gpio_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN;
    (void)RCC->AHB1ENR;

    GPIOC->BSRR = SET(PIN_OE);                       /* OE apagado antes de ser salida */
    GPIOC->BSRR = RST(PIN_CLK) | (DATA_MASK << 16);
    lat_level(0);

    for (uint32_t i = 0; i < N_OUT; i++) {
        uint32_t p = out_pins[i];
        GPIOC->PUPDR   &= ~(3u << 2*p);                                  /* sin pull interno */
        GPIOC->MODER    = (GPIOC->MODER   & ~(3u << 2*p)) | (1u << 2*p); /* 01 = salida */
        GPIOC->OSPEEDR  = (GPIOC->OSPEEDR & ~(3u << 2*p)) | (2u << 2*p); /* 10 = alta */
#if OPEN_DRAIN
        GPIOC->OTYPER  |=  (1u << p);   /* 1 = drenador abierto: '1' deja el pin flotando */
#else
        GPIOC->OTYPER  &= ~(1u << p);   /* 0 = push-pull */
#endif
    }

    GPIOA->MODER = (GPIOA->MODER & ~(3u << 2*5)) | (1u << 2*5);          /* PA5 = LD2 */
    GPIOA->MODER  = (GPIOA->MODER & ~(3u << 2*2)) | (2u << 2*2);         /* PA2 = AF */
    GPIOA->AFR[0] = (GPIOA->AFR[0] & ~(0xFu << 4*2)) | (7u << 4*2);      /* AF7 = USART2_TX */
}

/* ===================== Autodiagnóstico de pines =====================
 * Para cada pin del HUB75 (antes de volverlo salida):
 *  1) Entrada con pull-DOWN interno (~40 kOhm). Si se lee 1, algo EXTERNO tira
 *     el pin hacia arriba (un pull-up en el panel, típico 10 kOhm a 5 V).
 *  2) Entrada con pull-UP interno. Si se lee 0, hay un pull-down externo o un corto a GND.
 *  3) Salida en ALTO 20 us y leer IDR. Si da 0, el pin está en CORTO a GND
 *     (una resistencia de más de ~100 Ohm dejaría leer 1).
 * Así sabemos si el panel tiene resistencias de pull-up en sus entradas: si las
 * tiene, el modo drenador abierto (VAR 12) daría niveles de 5 V sin hardware extra. */
static uint32_t read_with_pull(uint32_t p, uint32_t pull)   /* pull: 1=up 2=down */
{
    GPIOC->MODER &= ~(3u << 2*p);                                /* 00 = entrada */
    GPIOC->PUPDR  = (GPIOC->PUPDR & ~(3u << 2*p)) | (pull << 2*p);
    delay_us(200);                                               /* que se cargue la capacidad del cable */
    uint32_t v = (GPIOC->IDR >> p) & 1u;
    GPIOC->PUPDR &= ~(3u << 2*p);
    return v;
}

static void pin_diag(void)
{
    /* Nombres según el cableado PREVISTO (README.md), sin importar la variante */
    static const uint8_t pins[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 11};
    static const char *const nm[] = {"R1", "G1", "B1", "R2", "G2", "B2", "OE", "LAT", "CLK", "A", "B"};
    uart_puts("Diag pines (pull-down -> lee / pull-up -> lee):\r\n");
    for (uint32_t i = 0; i < sizeof pins; i++) {
        uint32_t p = pins[i];
        uint32_t d = read_with_pull(p, 2), u = read_with_pull(p, 1);
        uart_puts("  "); uart_puts(nm[i]); uart_puts(" PC"); uart_putu(p);
        uart_puts(": PD="); uart_putu(d); uart_puts(" PU="); uart_putu(u);
        /* Prueba de corto: manejar el pin en ALTO solo 20 us y leer IDR.
           Si IDR da 0 con la salida en 1, el pin está en corto a GND. */
        GPIOC->BSRR   = SET(p);
        GPIOC->MODER  = (GPIOC->MODER & ~(3u << 2*p)) | (1u << 2*p);
        delay_us(20);
        uint32_t o = (GPIOC->IDR >> p) & 1u;
        GPIOC->MODER &= ~(3u << 2*p);
        GPIOC->BSRR   = RST(p);
        uart_puts(" OUT1="); uart_putu(o);
        uart_puts(d ? "  <- PULL-UP EXTERNO" : (!u ? "  <- PULL-DOWN EXTERNO / CORTO" : ""));
        uart_puts("\r\n");
    }
}

/* ===================== Envío genérico de una "palabra" por chip =====================
 * Desplaza nbits pulsos; cada chip (16 salidas) recibe word MSB primero, el mismo
 * valor en las 6 líneas. LAT queda activo durante los ÚLTIMOS lat_clocks pulsos:
 * así codifican sus comandos los drivers "inteligentes" (FM6126A, ICN2053...). */
__attribute__((unused))   /* no todas las variantes la usan */
static void send_word_cmd(uint16_t word, uint32_t nbits, uint32_t lat_clocks)
{
    for (uint32_t k = 0; k < nbits; k++) {
        uint32_t bit = (word >> (15 - (k % 16))) & 1u;
        if (k == nbits - lat_clocks) lat_level(1);
        clk_bit(bit ? DATA_MASK : 0);
    }
    lat_level(0);
    GPIOC->BSRR = RST(PIN_CLK);
    delay_us(2);
}

#if PWM_DRIVER
/* ===================== Intento ICN2053 / FM6353 =====================
 * Estos drivers tienen PWM interno y SRAM: no se "refrescan" por fase como un
 * 595. Comandos según el número de CLK con LAT en alto (LE) - tomado de
 * librerías abiertas, sin datasheet oficial confirmado:
 *   1 = DATA_LATCH (guarda 16 bits de un canal)   3 = VSYNC (muestra el cuadro)
 *   14 = PRE_ACT (habilita escribir config)       4/6/8/10 = WR_REG1..4
 * El brillo lo genera el propio chip contando pulsos de GCLK, que en el
 * conector HUB75 llega por el pin OE. Hay que dar GCLK continuamente. */
#define ICN_CHIPS (NBITS / 16)   /* chips en cadena por línea de color */
static void icn_init(void)
{
    static const uint16_t reg[4] = {0x1F70, 0x6707, 0x40F7, 0x0040}; /* valores típicos de proyectos abiertos */
    static const uint8_t  wr[4]  = {4, 6, 8, 10};
    for (int i = 0; i < 4; i++) {
        send_word_cmd(0, NBITS, 14);              /* PRE_ACT */
        send_word_cmd(reg[i], NBITS, wr[i]);      /* WR_REGi */
    }
}
static void icn_frame(void)
{
    for (uint32_t row = 0; row < 4; row++)        /* 4 filas de escaneo */
        for (uint32_t ch = 0; ch < 16; ch++) {    /* 16 canales por chip */
            for (uint32_t k = 0; k < NBITS; k++) {  /* 16 bits de gris x ICN_CHIPS chips */
                if (k == NBITS - 1) lat_level(1); /* DATA_LATCH = 1 CLK con LE */
                clk_bit(RED);                     /* gris 0xFFFF -> máximo en rojo */
            }
            lat_level(0);
        }
    send_word_cmd(0, NBITS, 3);                   /* VSYNC */
}
#endif

/* startup.c pone TIM3_IRQHandler en la tabla de vectores; aquí TIM3 no se usa,
   pero el símbolo debe existir para que enlace. */
void TIM3_IRQHandler(void) { }

int main(void)
{
    dwt_init();
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN;
    (void)RCC->AHB1ENR;
    uart_init();
    GPIOA->MODER  = (GPIOA->MODER & ~(3u << 2*2)) | (2u << 2*2);
    GPIOA->AFR[0] = (GPIOA->AFR[0] & ~(0xFu << 4*2)) | (7u << 4*2);

    uart_puts("\r\n=== PRUEBAS HUB75  VAR="); uart_putu(VAR);
    uart_puts(": " DESC);
    uart_puts(MIRROR ? "  [MIRROR] ===\r\n" : " ===\r\n");
    pin_diag();
    gpio_init();

#if FM6126A
    /* Igual que en main.c pero con REG1=7FFF (corriente máxima). OE apagado. */
    send_word_cmd(0x7FFF, NBITS, 12);
    send_word_cmd(0x0040, NBITS, 13);
#endif

#if PWM_DRIVER
    icn_init();
    icn_frame();
    uint32_t n = 0;
    while (1) {
        /* GCLK: OE oscila sin parar; cada 138 pulsos se cambia de fila (A/B). */
        for (uint32_t row = 0; row < 4; row++) {
            set_rows(row);
            for (uint32_t g = 0; g < 138; g++) {
                GPIOC->BSRR = SET(PIN_OE);
                GPIOC->BSRR = RST(PIN_OE);
            }
        }
        if (++n % 2000 == 0) { GPIOA->ODR ^= (1u << 5); icn_frame(); }   /* reenviar cuadro de vez en cuando */
    }
#elif VAR == 17
    uint16_t lfsr = 0xACE1u;
    uint32_t n = 0;
    while (1) {
        /* LFSR de Galois (polinomio 0xB400): genera 65535 valores antes de repetirse */
        lfsr = (lfsr >> 1) ^ ((lfsr & 1u) ? 0xB400u : 0u);
        uint32_t pins = (1u<<0) | (1u<<1) | (1u<<3) | (1u<<4) | (1u<<7) | (1u<<8);
        uint32_t v = 0;                    /* repartir 6 bits del LFSR en esos 6 pines */
        if (lfsr & 0x01) v |= 1u << 0;
        if (lfsr & 0x02) v |= 1u << 1;
        if (lfsr & 0x04) v |= 1u << 3;
        if (lfsr & 0x08) v |= 1u << 4;
        if (lfsr & 0x10) v |= 1u << 7;
        if (lfsr & 0x20) v |= 1u << 8;
        GPIOC->BSRR = v | ((~v & pins) << 16);
        delay_us(1);
        if ((++n & 0x1FF) == 0) set_rows((n >> 9) & 3);   /* cambiar fila cada ~0.5 ms */
        if ((n & 0x3FFFF) == 0) GPIOA->ODR ^= (1u << 5);
    }
#elif VAR == 18
    /* Puntos elegidos: bordes y saltos de potencia de 2 en la fase 0 (para ver
       cómo avanza la cadena) y algunos bits de las demás fases (para ver qué
       filas maneja cada fase). Formato: {fase, bit}. */
    static const uint8_t pts[][2] = {
        {0, 0}, {0, 1}, {0, 2}, {0, 7}, {0, 8}, {0, 15}, {0, 16}, {0, 23}, {0, 24},
        {0, 31}, {0, 32}, {0, 47}, {0, 48}, {0, 63},
        {1, 0}, {1, 63}, {2, 0}, {2, 63}, {3, 0}, {3, 63},
    };
    const uint32_t npts = sizeof pts / sizeof pts[0];
    uint32_t idx = 0, t_paso = DWT_CYCCNT;
    uart_puts("PASO fase=0 bit=0\r\n");
    while (1) {
        for (uint32_t f = 0; f < 4; f++) {
            oe_set(0);
            for (uint32_t k = 0; k < NBITS; k++)          /* solo el bit elegido en 1 */
                clk_bit((f == pts[idx][0] && k == pts[idx][1]) ? ((1u << 0) | (1u << 5)) : 0);
            GPIOC->BSRR = RST(PIN_CLK);
            latch_pulse();
            set_rows(f);
            oe_set(1);
            delay_us(500);
        }
        /* DWT a 16 MHz desborda cada ~268 s; la resta sin signo lo tolera */
        if (DWT_CYCCNT - t_paso >= 4000000u * CPU_MHZ) {
            t_paso = DWT_CYCCNT;
            idx = (idx + 1) % npts;
            GPIOA->ODR ^= (1u << 5);
            uart_puts("PASO fase="); uart_putu(pts[idx][0]);
            uart_puts(" bit="); uart_putu(pts[idx][1]); uart_puts("\r\n");
        }
    }
#elif VAR == 19
    /* raw[f][k]: valor de las 6 líneas de color en el pulso k de la fase f (igual que main.c) */
    static uint8_t raw[4][NBITS];
    for (uint32_t y = 0; y < 16; y++)
        for (uint32_t x = 0; x < 32; x++) {
            uint32_t c = 0;                                   /* color: bit0=R bit1=G bit2=B */
            if (x == 0 || x == 31 || y == 0 || y == 15) c = 2;  /* marco verde */
            if (x == y)      c = 1;                            /* diagonal roja */
            if (x == y + 16) c = 4;                            /* diagonal azul */
            /* --- fórmula inversa del mapeo (ver MAPEO.md) --- */
            uint32_t half = y / 8;            /* 0 = mitad superior (R1 G1 B1), 1 = inferior (R2 G2 B2) */
            uint32_t yy   = y % 8;
            uint32_t f    = yy % 4;           /* fase = valor de A/B */
            uint32_t j    = (yy < 4) ? 7 - (x % 8)   /* filas 0-3: primeros 8 bits del bloque, al revés */
                                     : 8 + (x % 8);  /* filas 4-7: últimos 8 bits, al derecho */
            uint32_t k    = 16 * (x / 8) + j; /* bloque de 16 bits por cada 8 columnas */
            raw[f][k] |= (uint8_t)(c << (3 * half));  /* R1G1B1 = PC0-2, R2G2B2 = PC3-5 */
        }
    while (1) {
        for (uint32_t f = 0; f < 4; f++) {
            oe_set(0);
            for (uint32_t k = 0; k < NBITS; k++) clk_bit(raw[f][k]);
            GPIOC->BSRR = RST(PIN_CLK);
            latch_pulse();
            set_rows(f);
            oe_set(1);
            delay_us(500);
        }
    }
#elif VAR == 16
    set_rows(ROW);
    uart_puts("A/B fijos en fila "); uart_putu(ROW); uart_puts("\r\n");
    while (1) { delay_us(250000); GPIOA->ODR ^= (1u << 5); }
#else
    uint32_t frames = 0;
    while (1) {
        for (uint32_t f = 0; f < 4; f++) {
            oe_set(0);                     /* apagar mientras se cambian datos y filas */
            for (uint32_t k = 0; k < NBITS; k++)
                clk_bit(RED);
            GPIOC->BSRR = RST(PIN_CLK);
            latch_pulse();                 /* registro de desplazamiento -> salidas */
            set_rows(f);
            oe_set(1);
            delay_us(500);                 /* tiempo visible de la fase */
        }
        if (++frames % 100 == 0) GPIOA->ODR ^= (1u << 5);   /* latido LD2 */
    }
#endif
}
