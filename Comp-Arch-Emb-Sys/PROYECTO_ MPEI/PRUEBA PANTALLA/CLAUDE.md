# Proyecto final MPEI – Snake en matriz LED 32x32 (Daniel)

## Reglas del curso (obligatorias)
- MCU Cortex-M4: **Nucleo-64 STM32F446RE**. Bare metal, toolchain GNU (arm-none-eabi-gcc + Makefile + linker script propio).
- Prohibido HAL/CMSIS/IDE comercial: los drivers de periféricos del MCU los escribimos nosotros (registros a mano en `regs.h`).
- Daniel debe poder explicar TODO el código: comentar el porqué de cada registro/bit, sin magia.
- Todo uso de IA se documenta en `AI_USO.md` (qué se generó y para qué).
- Responder en español, conciso y paso a paso.

## Hardware
- 2 módulos P10 RGB exterior "LKP10 全彩 4S" (Fervicom ref. 3050069), 32x16 px c/u, HUB75, escaneo 1/4 (A,B), 5 V (3 A máx. c/u).
  Se apilarán para 32x32. Por ahora solo 1 panel conectado al IN (flecha ↑ en el PCB: el IN es el conector de abajo).
- Chip driver: texto parcialmente legible "RR612…" (sin identificar).
- Control previsto: NES (CD4021).
- Programación: ST-LINK conectado a WSL (usbipd); se graba con `make flash` (st-flash).

## Conexión HUB75 (IN panel 1) -> Nucleo
R1=PC0 G1=PC1 B1=PC2 R2=PC3 G2=PC4 B2=PC5 · A=PC10 B=PC11 · CLK=PC8 · LAT=PC7 · OE=PC6 (TIM3_CH1 AF2, PWM de brillo) · GND.
Pines 4, 8, 11, 12 sin conectar. Fuente 5 V externa al panel, GND común con la Nucleo.
Extras: LD2=PA5, botón azul=PC13, USART2 TX=PA2 (VCP ST-LINK, 115200).

## Software en esta carpeta
- `startup.c` (vectores, copia .data, limpia .bss, FPU), `stm32f446.ld`, `regs.h`, `Makefile`.
- `main.c`: refresco HUB75 por ISR de TIM3 (500 us por fase, OE por PWM hardware), modos WALK/FASE/FILL con botón azul.
  Opción `USE_FM6126A_INIT`.
- `scan.c`: barre 12 configs (sin init / FM6126A 07E0-0040 / FM6126A 7FFF-0040) x OE bajo/alto x CLK subida/bajada.
  `make scan && make flash-scan`.

## Estado / diagnóstico (22-oct-2026)
- LD2 parpadea -> firmware e ISR de TIM3 corren.
- Panel recibe 5 V. Cable en el IN, pines verificados.
- (Histórico, ya resuelto abajo) NO encendía nada con main.c ni con ninguna de las 12 configs de scan.c.
- Hipótesis pendientes, en orden:
  1. Nivel lógico 3.3 V insuficiente -> poner 2x 74HCT245 (VCC 5 V, DIR=5 V, /OE=GND, A<-STM, B->HUB75).
  2. Señales no llegan al conector (medir CLK pin 13 / LAT pin 14 con osciloscopio).
  3. Driver tipo PWM (ICN2053/MBI5153/FM6353), protocolo distinto -> identificar el chip con foto nítida.
- Mapeo de píxeles (x,y) aún por descubrir con el modo WALK una vez encienda.

## Diagnóstico 22-oct-2026 (tarde) – ver PRUEBAS.md
- PC2 (B1), PC5 (B2) y PC6 (OE) en CORTO a GND (leen 0 aun manejados en alto). OE del panel siempre activo.
- A/B SÍ llegan y el panel reconoce 3.3 V (VAR 16 mueve el LED de fila) -> nivel lógico NO es la causa principal.
- CLK/LAT sin efecto con 15 variantes x cable normal/espejado + fuzz LFSR -> fallo de conexión en CLK/LAT.
- Siguiente paso: diagnóstico con el panel desconectado + continuidad de pines 13/14/15. `pruebas.c` deja PC2/5/6 como entradas.

## RESUELTO 22-oct-2026 (2.a sesión) – ver PRUEBAS.md §6-9 y MAPEO.md
- Causa: filas del conector HUB75 invertidas (B1/B2/OE caían en GND 4/8/16). Daniel movió los jumpers.
- Autodiagnóstico sin cortos; `PINES_EN_CORTO 0` en `pruebas.c`.
- `main.c` funciona tal cual: WALK se mueve, FILL pinta R/G/B completo. No hace falta 74HCT245.
- Mapeo medido y validado (VAR 19): bloque=bit/16, j=bit%16; j<8: x=8*bloque+7-j, y=fase;
  j>=8: x=8*bloque+j-8, y=fase+4; R2/G2/B2 -> y+8. Siguiente: driver de framebuffer (x,y) para el Snake.
- `serpiente.c`: demo grabada en la placa (serpiente de 24 LEDs R,G,B en zig-zag, TIM3 + doble buffer + set_pixel con el mapeo).
