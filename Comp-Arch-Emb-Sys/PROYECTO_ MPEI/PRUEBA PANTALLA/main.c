/*
 * main.c - Serpiente que recorre el panel P10 RGB (32x16) en zig-zag.
 *
 * La cabeza es ROJA y el cuerpo VERDE. La serpiente avanza un píxel por paso:
 * fila 0 de izquierda a derecha, fila 1 de derecha a izquierda, y así hasta
 * la fila 15; luego vuelve a empezar.
 *
 * Todo se hace en el lazo principal, sin interrupciones:
 *   dibujar la serpiente -> refrescar el panel varias veces -> avanzar un paso.
 */
#include <stdint.h>
#include "inc/stm32f446.h"
#include "inc/gpio_config.h"
#include "inc/panel_hub75.h"

#define LARGO_SERPIENTE   10   // número de píxeles de la serpiente
#define CUADROS_POR_PASO  25   // refrescos del panel antes de avanzar (más = más lenta)
#define TOTAL_POSICIONES  (PANEL_ANCHO * PANEL_ALTO)   // 32 * 16 = 512 posiciones

/* Convierte una posición del recorrido (0..511) en columna y fila.
 * Filas pares: de izquierda a derecha. Filas impares: de derecha a izquierda. */
static void posicion_a_columna_fila(uint16_t posicion, uint8_t *columna, uint8_t *fila)
{
    *fila = posicion / PANEL_ANCHO;

    if (*fila % 2 == 0) {
        *columna = posicion % PANEL_ANCHO;
    } else {
        *columna = (PANEL_ANCHO - 1) - (posicion % PANEL_ANCHO);
    }
}

int main(void)
{
    GPIO_Config();

    uint16_t cabeza = 0;   // posición de la cabeza en el recorrido

    while (1) {
        /* 1) Dibujar la serpiente en la imagen */
        Panel_Limpiar();

        for (uint8_t segmento = 0; segmento < LARGO_SERPIENTE; segmento++) {
            // cada segmento va una posición detrás del anterior;
            // se suma TOTAL_POSICIONES para que al restar no quede negativo
            uint16_t posicion = (cabeza + TOTAL_POSICIONES - segmento) % TOTAL_POSICIONES;
            uint8_t columna;
            uint8_t fila;
            posicion_a_columna_fila(posicion, &columna, &fila);

            if (segmento == 0) {
                Panel_PonerPixel(columna, fila, COLOR_ROJO);    // cabeza
            } else {
                Panel_PonerPixel(columna, fila, COLOR_VERDE);   // cuerpo
            }
        }

        /* 2) Mostrar la imagen varias veces (esto también da el tiempo de cada paso) */
        for (uint8_t cuadro = 0; cuadro < CUADROS_POR_PASO; cuadro++) {
            Panel_Refrescar();
        }

        /* 3) Avanzar la cabeza una posición (al llegar a 512 vuelve a 0) */
        cabeza = (cabeza + 1) % TOTAL_POSICIONES;
    }
}
