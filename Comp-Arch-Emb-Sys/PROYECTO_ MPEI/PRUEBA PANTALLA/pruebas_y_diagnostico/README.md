# PRUEBA PANTALLA – P10 RGB HUB75 con STM32F446RE (bare metal)

Programa de prueba para verificar el cableado del módulo P10 RGB 4S (32×16, HUB75, escaneo 1/4)
y descubrir su mapeo interno antes de escribir el driver definitivo del Snake.

## Archivos
| Archivo | Qué es |
|---|---|
| `regs.h` | Registros del F446RE definidos a mano (RCC, GPIO, TIM3, USART2, NVIC) |
| `startup.c` | Tabla de vectores + `Reset_Handler` (copia `.data`, limpia `.bss`, habilita FPU) |
| `stm32f446.ld` | Linker script: FLASH 512 KB @0x08000000, RAM 128 KB @0x20000000 |
| `main.c` | Refresco HUB75 por interrupción de TIM3 + modos de prueba |
| `Makefile` | Compilación con `arm-none-eabi-gcc`, grabación con `st-flash` u OpenOCD |
| `../AI_USO.md` | Declaración del uso de IA (requisito 10) |
| `pruebas.c` | Banco de pruebas del protocolo (variantes con `make pruebas VAR=n`), autodiagnóstico de pines |
| `PRUEBAS.md` | Registro de pruebas del 22-oct y conclusiones |
| `MAPEO.md` | Mapeo medido (fase, bit) → (x, y) del panel y fórmula inversa |
| `serpiente.c` | Demo: serpiente R/G/B que recorre el panel en zig-zag (`make serpiente && make flash-serpiente`) |

## Conexión (HUB75 IN del panel 1 → Nucleo-F446RE)
| HUB75 | Señal | Pin | Conector Nucleo |
|---|---|---|---|
| 1 | R1 | PC0 | CN7-38 |
| 2 | G1 | PC1 | CN7-36 |
| 3 | B1 | PC2 | CN7-35 |
| 5 | R2 | PC3 | CN7-37 |
| 6 | G2 | PC4 | CN10-34 |
| 7 | B2 | PC5 | CN10-6 |
| 9 | A | PC10 | CN7-1 |
| 10 | B | PC11 | CN7-2 |
| 13 | CLK | **PC8** | CN10-2 |
| 14 | LAT | PC7 | CN10-19 |
| 15 | OE | **PC6** | CN10-4 |
| 16 | GND | GND | CN7-8 |

OE va en PC6 porque es la salida TIM3_CH1: el brillo (PWM de OE) lo genera el timer por hardware.

Alimentación: fuente de 5 V a los cables rojo/negro del panel, y **GND de la fuente unido al GND de la Nucleo**.

## Compilar y grabar (WSL)
```bash
make clean
make              # 1 panel
make N_PANELS=2   # 2 paneles en cadena
make flash        # st-flash (o: make flash-ocd)
```

Ver mensajes UART (Virtual COM del ST-LINK, 115200 8N1). Desde Windows se puede usar PuTTY o
el Serial Monitor de VS Code en el puerto COM de la Nucleo.

## Modos (botón azul B1 para cambiar)
1. **WALK**: un solo bit recorre toda la cadena; rojo en la mitad superior, azul en la inferior.
   La UART imprime `fase=F (B= A=) bit=K`. Anoten (fase, bit) → (x, y) para armar la tabla de mapeo.
2. **FASE**: enciende todas las columnas de una fase. Muestra qué filas selecciona cada combinación de A/B.
3. **FILL**: pantalla completa en rojo, verde y azul con brillo al 15 %.

El LED verde LD2 parpadea a 2 Hz si el programa está corriendo.

## Si algo no se ve bien
| Síntoma | Ajuste en `main.c` |
|---|---|
| Todo encendido donde debería estar apagado (negativo) | `DATA_ACTIVE_HIGH 0` |
| Nada enciende, o se ve al revés el brillo | `OE_ACTIVE_LOW 0` |
| En WALK el bit "se sale" antes o tarda en aparecer | revisar `BITS_PER_PANEL` (64 esperado para 1/4 scan) |
| Filas repetidas en 2 sitios en modo FASE | el panel puede ser 1/8 scan (necesitaría la línea C) |
