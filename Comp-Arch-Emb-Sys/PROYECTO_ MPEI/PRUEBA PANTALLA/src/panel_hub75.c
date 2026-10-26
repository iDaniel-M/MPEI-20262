/*
 * panel_hub75.c - Driver del panel P10 RGB 32x16 (HUB75, escaneo 1/4).
 *
 * Cómo funciona el panel:
 *  - El panel NO guarda una imagen: solo puede encender 4 filas de arriba y 4 de abajo
 *    a la vez (una "fase"). Hay 4 fases (A y B eligen cuál: fase = B*2 + A).
 *  - Mostrando las 4 fases muy rápido una tras otra, el ojo ve la imagen completa.
 *  - Para cada fase se envían 64 datos en serie: en cada pulso de CLK el panel guarda
 *    los 6 pines de color (R1 G1 B1 de la mitad de arriba, R2 G2 B2 de la de abajo).
 *  - Un pulso de LAT pasa los 64 datos a los LEDs, y OE = 0 los enciende.
 */
#include "panel_hub75.h"
#include "gpio_config.h"
#include "SysTick.h"

#define NUMERO_FASES      4    // escaneo 1/4
#define PULSOS_POR_FASE   64   // datos que se envían en cada fase
/* Brillo: 40 % del original (antes 150 us encendido). Encendido + apagado = 150 us,
   así cada fase dura lo mismo y la velocidad de la serpiente no cambia. */
#define TIEMPO_ENCENDIDO_US 60  // tiempo que se encienden los LEDs en cada fase (brillo)
#define TIEMPO_APAGADO_US   90  // tiempo extra con los LEDs apagados

/* La imagen: un color por píxel. pantalla[fila][columna] */
static uint8_t pantalla[PANEL_ALTO][PANEL_ANCHO];

void Panel_Limpiar(void)
{
    for (uint8_t fila = 0; fila < PANEL_ALTO; fila++) {
        for (uint8_t columna = 0; columna < PANEL_ANCHO; columna++) {
            pantalla[fila][columna] = COLOR_APAGADO;
        }
    }
}

void Panel_PonerPixel(uint8_t columna, uint8_t fila, uint8_t color)
{
    if (columna < PANEL_ANCHO && fila < PANEL_ALTO) {   // ignorar puntos fuera del panel
        pantalla[fila][columna] = color;
    }
}

/*
 * Envía los 64 datos de una fase. Para saber qué píxel corresponde a cada pulso se usa
 * el mapeo medido (ver MAPEO.md):
 *   - Cada 16 pulsos forman un bloque de 8 columnas: bloque = pulso / 16.
 *   - Los primeros 8 pulsos del bloque son la fila "fase", de DERECHA a IZQUIERDA.
 *   - Los últimos 8 pulsos del bloque son la fila "fase + 4", de IZQUIERDA a DERECHA.
 *   - Con los mismos pulsos, R2/G2/B2 dibujan la fila 8 más abajo (mitad de abajo).
 */
static void enviar_fase(uint8_t fase)
{
    for (uint8_t pulso = 0; pulso < PULSOS_POR_FASE; pulso++) {
        uint8_t bloque   = pulso / 16;   // 0..3
        uint8_t posicion = pulso % 16;   // 0..15 dentro del bloque
        uint8_t columna;
        uint8_t fila;

        if (posicion < 8) {
            columna = 8 * bloque + (7 - posicion);
            fila    = fase;
        } else {
            columna = 8 * bloque + (posicion - 8);
            fila    = fase + 4;
        }

        uint8_t color_arriba = pantalla[fila][columna];      // va por R1 G1 B1 (bits 0..2)
        uint8_t color_abajo  = pantalla[fila + 8][columna];  // va por R2 G2 B2 (bits 3..5)

        write_color_pins(color_arriba | (color_abajo << 3));

        write_pin_state(GPIOC, PIN_CLK, 1);   // subida de CLK: el panel guarda el dato
        write_pin_state(GPIOC, PIN_CLK, 0);
    }
}

/* Muestra la imagen una vez completa: las 4 fases. */
void Panel_Refrescar(void)
{
    for (uint8_t fase = 0; fase < NUMERO_FASES; fase++) {
        enviar_fase(fase);                     // 1) mandar los 64 datos (los LEDs siguen apagados)

        write_pin_state(GPIOC, PIN_OE, 1);     // 2) LEDs apagados mientras se cambia de fila
        write_pin_state(GPIOC, PIN_A, fase & 1);         // 3) elegir las filas: A = bit 0 de la fase
        write_pin_state(GPIOC, PIN_B, (fase >> 1) & 1);  //                      B = bit 1 de la fase

        write_pin_state(GPIOC, PIN_LAT, 1);    // 4) pulso de LAT: los datos pasan a los LEDs
        write_pin_state(GPIOC, PIN_LAT, 0);

        write_pin_state(GPIOC, PIN_OE, 0);     // 5) encender los LEDs un rato
        SysTick_Delay_us(TIEMPO_ENCENDIDO_US);
        write_pin_state(GPIOC, PIN_OE, 1);     // 6) apagar antes de la siguiente fase
        SysTick_Delay_us(TIEMPO_APAGADO_US);   // 7) esperar apagado (baja el brillo)
    }
}
