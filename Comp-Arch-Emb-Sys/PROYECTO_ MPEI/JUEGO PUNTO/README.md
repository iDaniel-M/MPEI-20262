# JUEGO PUNTO – un píxel que se mueve en el panel P10 (STM32F446RE)

Basado en el ejercicio **21-SPI-4** del profesor (mismos `crt0.S`, `linker_script.ld`, `Makefile`,
`gpio_config`, `system_config`, `usart` y estilo de `main.c`). El MAX7219 se cambió por el panel
P10 RGB 32x16 (`src/p10.c`).

## Compilar y grabar (WSL, ST-LINK conectado con usbipd)
```
make          # compila main.elf y main.bin
make flash    # graba con st-flash
make clean
```
UART: Virtual COM del ST-LINK, 115200 8N1 (en WSL: `/dev/ttyACM0`). Imprime `x=..,y=..` cada 200 ms.

## Controles
| Botón del shield | Pin | Tecla UART | Acción |
|---|---|---|---|
| A | PA10 | `w` | arriba (y − 1) |
| B | PB3 | `d` | derecha (x + 1) |
| C | PB5 | `s` | abajo (y + 1) |
| D | PB4 | `a` | izquierda (x − 1) |
| E | PB10 | `p` | pausa |
| F | PA8 | `r` | reiniciar: punto al centro (16, 8) y quieto |

Botones activos en bajo (pull-up interno). x = 0..31, y = 0..15 (y = 0 arriba); el punto no sale del borde.

## Archivos
| Archivo | Qué es |
|---|---|
| `crt0.S`, `linker_script.ld` | Del profesor. Cambios: sin `SystemInit` (CMSIS) y RAM de 128 KB |
| `inc/stm32f446_regs.h` | Registros escritos a mano con los mismos nombres de CMSIS (CMSIS está prohibido en el curso) |
| `inc/system_config.h`, `src/system_config.c` | Del profesor: reloj a 84 MHz (PLL), SysTick de 1 ms y sw timers. Se corrigió `WRITE_REG_FIELD` |
| `inc/gpio_config.h`, `src/gpio_config.c` | Del profesor: `GPIO_Config`, `read/write/toggle_pin_state` |
| `inc/usart.h`, `src/usart.c` | Del profesor: USART2 en PA2/PA3 |
| `inc/p10.h`, `src/p10.c` | Driver del panel: refresco por TIM3 (PSC 83 a 84 MHz), OE por PWM en PC6, mapeo de `PRUEBA PANTALLA/MAPEO.md` |
| `main.c` | El juego |

## Conexión del panel (HUB75 IN)
R1=PC0 G1=PC1 B1=PC2 R2=PC3 G2=PC4 B2=PC5 · OE=PC6 · LAT=PC7 · CLK=PC8 · A=PC10 B=PC11 · GND.
