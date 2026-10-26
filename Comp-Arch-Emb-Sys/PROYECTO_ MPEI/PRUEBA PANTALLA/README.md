# Serpiente en panel P10 RGB 32x16 (STM32F446RE, bare metal)

Misma estructura que los ejemplos del curso (`5-ARM-Boards/NUCLEO-64` de
[Comp-Arch-Emb-Sys](https://github.com/divadnauj-GB/Comp-Arch-Emb-Sys)): arranque en ensamblador,
linker script propio, registros escritos a mano y un driver por periférico. Sin HAL ni CMSIS.

## Archivos
| Archivo | Qué hace |
|---|---|
| `crt0.S` | Arranque en ensamblador: tabla de vectores, copia `.data`, limpia `.bss`, llama a `main` |
| `linker_script.ld` | Dónde va cada sección en FLASH (512 KB) y RAM (128 KB) |
| `inc/stm32f446.h` | Registros RCC, GPIO y SysTick (structs + direcciones del manual) |
| `inc/gpio_config.h`, `src/gpio_config.c` | Pines del HUB75 y funciones para escribirlos |
| `inc/SysTick.h`, `src/SysTick.c` | Espera en microsegundos con el SysTick (polling) |
| `inc/panel_hub75.h`, `src/panel_hub75.c` | Imagen 32x16 en RAM y refresco del panel (usa el mapeo de `MAPEO.md`) |
| `main.c` | La serpiente: dibujar, refrescar, avanzar |
| `MAPEO.md` | Cómo se midió qué pulso de CLK corresponde a cada píxel |
| `pruebas_y_diagnostico/` | Todo lo anterior: pruebas del panel, autodiagnóstico de pines, fotos, demo vieja |

## Compilar y grabar
```
make
make flash
```

## Conexión (HUB75 IN -> Nucleo)
R1=PC0 G1=PC1 B1=PC2 R2=PC3 G2=PC4 B2=PC5 · OE=PC6 · LAT=PC7 · CLK=PC8 · A=PC10 B=PC11 · GND.
Fuente de 5 V aparte para el panel, con GND común a la Nucleo.
Si el panel no responde: `cd pruebas_y_diagnostico && make pruebas VAR=0 && make flash-pruebas`
y leer el autodiagnóstico de pines por la UART (ver `pruebas_y_diagnostico/PRUEBAS.md` §11).
