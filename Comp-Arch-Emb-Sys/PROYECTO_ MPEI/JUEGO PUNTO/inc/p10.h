#ifndef _P10_H_
#define _P10_H_

#include "system_config.h"

/* Panel P10 RGB 32x16 (HUB75, escaneo 1/4). Reemplaza al max7219 del profesor. */
#define P10_ANCHO 32   // x = 0..31 (izquierda -> derecha)
#define P10_ALTO  16   // y = 0..15 (arriba -> abajo)

/* Color de un píxel: bit0 rojo, bit1 verde, bit2 azul */
#define P10_ROJO  1
#define P10_VERDE 2
#define P10_AZUL  4

void p10_config(void);
void p10_clear(void);
void p10_set_pixel(uint8_t x, uint8_t y, uint8_t color);

#endif
