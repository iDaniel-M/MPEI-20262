# Declaración de uso de IA

Herramienta: Claude (Anthropic), en Claude Code.

| Fecha | Qué se generó / hizo con IA | Para qué | Revisión propia (Daniel) |
|---|---|---|---|
| 22-oct | Versión inicial de `regs.h`, `startup.c`, `stm32f446.ld`, `main.c`, `Makefile`, `README.md` | Programa de prueba del panel HUB75 | (completar) |
| 22-oct | `pruebas.c` (variantes del protocolo, autodiagnóstico de pines), registros DWT en `regs.h`, objetivos del `Makefile`, `PRUEBAS.md` | Diagnosticar por qué el panel no encendía | (completar) |
| 22-oct | Pruebas VAR 18/19, `MAPEO.md`; análisis de fotos con OpenCV (fuera del repo) | Medir y validar el mapeo (x, y) de los píxeles | (completar) |
| 22-oct | `serpiente.c` (TIM3 + doble buffer) | Primera demo | (completar) |
| 26-oct | Diagnóstico con webcam + UART + `pruebas.c` VAR 0 (sin cambios de código) | Encontrar los jumpers en GND (`PRUEBAS.md` §11) | (completar) |
| 26-oct | Serpiente actual: `crt0.S`, `linker_script.ld`, `inc/`, `src/`, `main.c`, `Makefile`, basada en los ejemplos del curso (divadnauj-GB/Comp-Arch-Emb-Sys, `NUCLEO-64/4-custom-hal` y `5-systick-delay`). Archivos de pruebas movidos a `pruebas_y_diagnostico/` | Versión simple y explicable del proyecto | (completar) |

Datos no verificados contra datasheet: los comandos de FM6126A / ICN2053 de `pruebas_y_diagnostico/`
vienen de librerías abiertas (no hay datasheet del chip del panel, "RR612…").
