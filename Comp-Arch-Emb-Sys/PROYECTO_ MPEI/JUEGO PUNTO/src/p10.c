/*
 * p10.c - Driver del panel P10 RGB 32x16 (HUB75, escaneo 1/4).
 * Misma lógica que PRUEBA PANTALLA (refresco por interrupción de TIM3, OE por PWM).
 *
 * Cómo se maneja el panel:
 *  - Solo enciende 1/4 de las filas a la vez (una "fase", elegida con A y B: fase = B*2 + A).
 *  - Por cada fase se envían 64 datos: en cada subida de CLK el panel guarda los 6 pines
 *    de color (R1 G1 B1 = mitad de arriba, R2 G2 B2 = mitad de abajo).
 *  - Un pulso de LAT pasa esos datos a los LEDs; OE = 0 los enciende.
 *  - TIM3 interrumpe cada 500 us y muestra la siguiente fase: 4 fases = 2 ms -> 500 imágenes/s.
 *
 * Pines (GPIOC): R1=PC0 G1=PC1 B1=PC2 R2=PC3 G2=PC4 B2=PC5
 *                OE=PC6 (TIM3_CH1, AF2)  LAT=PC7  CLK=PC8  A=PC10  B=PC11
 */
#include "p10.h"
#include "gpio_config.h"

#define PIN_OE  6
#define PIN_LAT 7
#define PIN_CLK 8
#define PIN_A   10
#define PIN_B   11
#define MASCARA_COLORES 0x3FU     // PC0..PC5 = 0b0011_1111

#define FASES    4                // escaneo 1/4
#define PULSOS   64               // datos (pulsos de CLK) por fase
#define FASE_US  500              // duración de cada fase en microsegundos
#define BRILLO   15               // % del tiempo con los LEDs encendidos

/* TIM3 recibe 84 MHz (2 x PCLK1, ver system_config.h).
   PSC = 83 -> el contador avanza a 84 MHz / (83+1) = 1 MHz -> 1 cuenta = 1 us.
   (Antes, con 16 MHz sin PLL, era PSC = 15.) */
#define TIM3_PSC 83

/* BSRR: escribir 1 en el bit n pone PCn en 1; en el bit n+16 lo pone en 0.
   Cambia solo esos pines y en una sola escritura (la interrupción no estorba al main). */
#define PIN_SET(p)   (1U << (p))
#define PIN_RESET(p) (1U << ((p) + 16))

/* Imagen en el orden en que se envía: buffer[fase][pulso] = 6 bits de color.
   volatile: la escribe el main y la lee la interrupción. */
static volatile uint8_t buffer[FASES][PULSOS];
static volatile uint8_t fase_cargada = 0;   // fase cuyos datos ya están dentro del panel

void p10_clear(void){
    for(uint8_t f=0; f<FASES; f++){
        for(uint8_t k=0; k<PULSOS; k++){
            buffer[f][k] = 0;
        }
    }
}

/* (x, y) -> (fase, pulso), fórmula medida en PRUEBA PANTALLA/MAPEO.md:
     - cada 8 columnas son un bloque de 16 pulsos: bloque = x / 8
     - filas 0-3 de cada mitad: primeros 8 pulsos del bloque, de derecha a izquierda
     - filas 4-7 de cada mitad: últimos 8 pulsos del bloque, de izquierda a derecha
     - mitad de abajo (y >= 8): mismos pulsos, pero por R2/G2/B2 (color << 3) */
void p10_set_pixel(uint8_t x, uint8_t y, uint8_t color){
    if(x >= P10_ANCHO || y >= P10_ALTO){
        return;                               // fuera del panel: no hacer nada
    }
    uint8_t mitad = y / 8;                    // 0 = arriba, 1 = abajo
    uint8_t fila  = y % 8;                    // fila dentro de la mitad (0..7)
    uint8_t fase  = fila % 4;
    uint8_t pulso;
    if(fila < 4){
        pulso = 16 * (x / 8) + (7 - x % 8);
    } else {
        pulso = 16 * (x / 8) + 8 + (x % 8);
    }
    buffer[fase][pulso] |= (color << (3 * mitad));
}

/* Envía los 64 datos de una fase (el panel aún muestra la anterior hasta el LAT) */
static void enviar_fase(uint8_t fase){
    for(uint8_t k=0; k<PULSOS; k++){
        uint32_t datos = buffer[fase][k];
        // pines en 1 = datos; pines en 0 = los de color que no están en datos; y CLK en 0
        GPIOC->BSRR = datos | ((~datos & MASCARA_COLORES) << 16) | PIN_RESET(PIN_CLK);
        GPIOC->BSRR = PIN_SET(PIN_CLK);       // subida de CLK: el panel guarda el dato
    }
    GPIOC->BSRR = PIN_RESET(PIN_CLK);
}

/* TIM3 llega a ARR cada 500 us. En ese momento el PWM apaga OE por hardware y lo vuelve
   a encender cuando el contador llega a CCR1. En ese rato apagado se hace el cambio: */
void TIM3_IRQHandler(void){
    TIM3->SR = 0;                             // borrar la bandera UIF (si no, vuelve a entrar)

    GPIOC->BSRR = PIN_SET(PIN_LAT);           // 1) LAT: mostrar los datos ya enviados
    GPIOC->BSRR = PIN_RESET(PIN_LAT);

    uint8_t f = fase_cargada;                 // 2) A y B = filas de esa fase
    GPIOC->BSRR = ((f & 1) ? PIN_SET(PIN_A) : PIN_RESET(PIN_A)) |
                  ((f & 2) ? PIN_SET(PIN_B) : PIN_RESET(PIN_B));

    f = (f + 1) % FASES;                      // 3) enviar ya la siguiente fase
    enviar_fase(f);
    fase_cargada = f;
}

void p10_config(void){
    GPIO_InitTypeDef GPIO_Init = {0};

    // OE en 1 (LEDs apagados) antes de volver salidas los pines
    GPIOC->BSRR = PIN_SET(PIN_OE);

    // PC0-PC5, LAT, CLK, A, B: salidas (Mode 1) rápidas (Speed 2), sin pull
    const uint8_t salidas[] = {0, 1, 2, 3, 4, 5, PIN_LAT, PIN_CLK, PIN_A, PIN_B};
    for(uint8_t i=0; i<sizeof(salidas); i++){
        GPIO_Init.Pin = salidas[i];
        GPIO_Init.Mode = 1;
        GPIO_Init.Speed = 2;
        GPIO_Config(GPIOC, GPIO_Init);
    }

    // PC6 = OE: función alterna (Mode 2) número 2 = TIM3_CH1 (datasheet, tabla "Alternate functions")
    GPIO_Init.Pin = PIN_OE;
    GPIO_Init.Mode = 2;
    GPIO_Init.Alternate = 2;
    GPIO_Config(GPIOC, GPIO_Init);

    p10_clear();
    enviar_fase(0);                           // dejar la fase 0 cargada para la 1.a interrupción

    // ---- TIM3 ----
    WRITE_REG_FIELD(RCC->APB1ENR, RCC_APB1ENR_TIM3EN, 1);  // encender el reloj de TIM3
    TIM3->PSC = TIM3_PSC;                     // 1 cuenta = 1 us
    TIM3->ARR = FASE_US - 1;                  // cuenta 0..499 -> 500 us por fase
    // Canal 1 en PWM modo 1 (OC1M = 110, bits 4-6) con preload (OC1PE, bit 3):
    // salida = 1 mientras CNT < CCR1 (OE = 1, apagado) y = 0 después (OE = 0, encendido)
    TIM3->CCMR1 = (6U << 4) | (1U << 3);
    TIM3->CCER = (1U << 0);                   // CC1E: sacar el canal 1 por PC6
    TIM3->CCR1 = FASE_US - (FASE_US * BRILLO / 100);   // encendido el último BRILLO %
    TIM3->CR1 = (1U << 7);                    // ARPE: ARR con preload
    TIM3->EGR = (1U << 0);                    // UG: cargar PSC/ARR/CCR1 ya
    TIM3->SR = 0;                             // borrar la bandera que deja UG
    TIM3->DIER = (1U << 0);                   // UIE: interrupción en cada desborde
    NVIC_EnableIRQ(TIM3_IRQn);                // habilitar la IRQ 29 en el NVIC
    TIM3->CR1 |= (1U << 0);                   // CEN: arrancar el contador
}
