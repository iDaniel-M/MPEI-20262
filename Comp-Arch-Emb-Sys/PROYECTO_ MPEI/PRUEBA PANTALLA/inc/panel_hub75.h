#ifndef __PANEL_HUB75_H__
#define __PANEL_HUB75_H__

#include "stm32f446.h"

#define PANEL_ANCHO 32   // columnas (x = 0..31, de izquierda a derecha)
#define PANEL_ALTO  16   // filas    (y = 0..15, de arriba hacia abajo)

/* Colores: 3 bits, uno por LED del píxel (bit0 rojo, bit1 verde, bit2 azul) */
#define COLOR_APAGADO  0
#define COLOR_ROJO     1   // 0b001
#define COLOR_VERDE    2   // 0b010
#define COLOR_AZUL     4   // 0b100
#define COLOR_AMARILLO 3   // rojo + verde
#define COLOR_BLANCO   7   // rojo + verde + azul

void Panel_Limpiar(void);
void Panel_PonerPixel(uint8_t columna, uint8_t fila, uint8_t color);
void Panel_Refrescar(void);

#endif /* __PANEL_HUB75_H__ */
