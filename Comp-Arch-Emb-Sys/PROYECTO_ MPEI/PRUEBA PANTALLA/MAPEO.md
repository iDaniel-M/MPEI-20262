# Mapeo del panel P10 RGB 4S (32×16, escaneo 1/4) – medido el 22-oct-2026

## Convención de coordenadas
- Panel visto **de frente** (lado de los LEDs), con la mitad que maneja **R1/G1/B1 arriba**.
- `x` = columna 0..31 de izquierda a derecha; `y` = fila 0..15 de arriba hacia abajo.
- `fase` = valor de las líneas de fila: `fase = B·2 + A` (0..3).
- `bit` = número de pulso de CLK dentro de la fase (0 = el primero que se envía, 63 = el último),
  igual que `raw[fase][bit]` en `main.c` y que la línea `fase=F bit=K` del modo WALK.

## Cómo se midió
1. `pruebas.c` VAR 18 (MAPA): el mismo patrón que WALK de `main.c` (un solo bit en 1: R1 rojo y
   B2 azul), pero se queda 4 s en cada (fase, bit) elegido e imprime `PASO fase= bit=` por UART.
   WALK normal avanza 3 pasos/s y una foto de la webcam tarda ~1.5 s, así que no se podía saber
   con certeza a qué paso correspondía cada foto.
2. Cada foto se asoció a su paso con la hora de la UART (`fotos/mapa/Fx_By.jpg`).
3. Posición del LED encendido: se calibró la cuadrícula de 32×16 con los LEDs apagados
   (homografía con OpenCV, 511/512 celdas detectadas, error medio 0.11 celdas) y se convirtió el
   centro del punto azul a (x, y). Se comprobó con dos calibraciones distintas (diferencia < 0.1 celda).
4. El rojo (R1) siempre aparece exactamente 8 filas arriba del azul (B2).

## Medidas

| fase | bit | x | y (R1/G1/B1) | y (R2/G2/B2) |
|---|---|---|---|---|
| 0 | 0 | 7 | 0 | 8 |
| 0 | 1 | 6 | 0 | 8 |
| 0 | 2 | 5 | 0 | 8 |
| 0 | 7 | 0 | 0 | 8 |
| 0 | 8 | 0 | 4 | 12 |
| 0 | 15 | 7 | 4 | 12 |
| 0 | 16 | 15 | 0 | 8 |
| 0 | 23 | 8 | 0 | 8 |
| 0 | 24 | 8 | 4 | 12 |
| 0 | 31 | 15 | 4 | 12 |
| 0 | 32 | 23 | 0 | 8 |
| 0 | 47 | 23 | 4 | 12 |
| 0 | 63 | 31 | 4 | 12 |
| 1 | 0 | 7 | 1 | 9 |
| 1 | 63 | 31 | 5 | 13 |
| 2 | 0 | 7 | 2 | 10 |
| 2 | 63 | 31 | 6 | 14 |
| 3 | 0 | 7 | 3 | 11 |
| 3 | 63 | 31 | 7 | 15 |

(Para fase 0 bit 48 no quedó foto dentro de la ventana de tiempo; la validación de abajo lo cubre.)

## Regla (fase, bit) → (x, y)

Los 64 bits de cada fase forman **4 bloques de 16** (un bloque por cada 8 columnas). En cada bloque:
los primeros 8 bits van a la fila `fase` **de derecha a izquierda**; los siguientes 8 van a la fila
`fase + 4` **de izquierda a derecha** (recorrido en "zig-zag" / "U").

```
bloque = bit / 16          j = bit % 16
si j < 8:   x = 8*bloque + (7 - j)     y = fase
si j >= 8:  x = 8*bloque + (j - 8)     y = fase + 4
líneas R1/G1/B1 -> y        líneas R2/G2/B2 -> y + 8
```

Dibujo de la fase 0 (número = bit; filas 0 y 4, mitad superior):

```
x:    0  1  2  3  4  5  6  7 |  8  9 10 11 12 13 14 15 | 16 ... 23 | 24 ... 31
y=0:  7  6  5  4  3  2  1  0 | 23 22 21 20 19 18 17 16 | 39 ... 32 | 55 ... 48
y=4:  8  9 10 11 12 13 14 15 | 24 25 26 27 28 29 30 31 | 40 ... 47 | 56 ... 63
```

Las fases 1, 2 y 3 son iguales, pero en las filas (1,5), (2,6) y (3,7).

## Fórmula inversa (la que usará el driver del Snake)

```c
/* (x,y) -> fase, bit y desplazamiento dentro del byte de 6 líneas de color */
half = y / 8;                  /* 0: R1G1B1 (PC0-2)   1: R2G2B2 (PC3-5) */
yy   = y % 8;
fase = yy % 4;
j    = (yy < 4) ? 7 - (x % 8) : 8 + (x % 8);
bit  = 16 * (x / 8) + j;
raw[fase][bit] |= color << (3 * half);   /* color: bit0=R bit1=G bit2=B */
```

## Validación
`pruebas.c` VAR 19 dibuja con esta fórmula inversa un marco verde en todo el borde, una diagonal roja
de (0,0) a (15,15) y una azul de (16,0) a (31,15). Foto: `fotos/V19_validacion_2.jpg`.
Las tres figuras salen completas y sin cortes, así que la regla vale para los 4 bloques, las
4 fases y las dos mitades.

Concuerda también con la prueba del 22-oct en la mañana (VAR 16, cable todavía con las filas invertidas
y por eso con A/B intercambiadas): la basura del latch aparecía en las filas 12, 13, 14 y 15 = `fase + 12`.
