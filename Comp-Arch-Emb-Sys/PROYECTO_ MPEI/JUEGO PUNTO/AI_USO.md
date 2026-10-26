# Declaración de uso de IA – JUEGO PUNTO

Herramienta: Claude (Anthropic), en Claude Code. 26-oct-2026.

| Qué se generó con IA | Para qué | Revisión propia (Daniel) |
|---|---|---|
| Copia de la base 21-SPI-4 del profesor y adaptación: quitar CMSIS (`inc/stm32f446_regs.h` escrito a mano, sin `SystemInit` en `crt0.S`), RAM de 128 KB en el linker, `Makefile` con st-flash | Usar la estructura del curso sin CMSIS | (completar) |
| Corrección de `WRITE_REG_FIELD` (`~(MASK)` con paréntesis) y `POSITION_VAL` con `__builtin_ctz` | El original borraba la configuración de pines vecinos | (completar) |
| `inc/p10.h`, `src/p10.c` (a partir del `main.c` de PRUEBA PANTALLA y de MAPEO.md), PSC de TIM3 y BRR de la UART recalculados para 84 MHz | Driver del panel en lugar del MAX7219 | (completar) |
| `main.c` (botones A–F, teclas w/a/s/d/p/r, paso cada 200 ms), `README.md` | El juego | (completar) |
| Prueba: grabar, comandos por UART y verificación con webcam | Comprobar que funciona | (completar) |
