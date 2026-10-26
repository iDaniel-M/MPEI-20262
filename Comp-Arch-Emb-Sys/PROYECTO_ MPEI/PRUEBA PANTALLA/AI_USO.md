# Declaración de uso de IA

- **Herramienta:** Claude (Anthropic).
- **Propósito:** generar un programa de prueba inicial para verificar el cableado del panel P10 RGB HUB75
  y descubrir su mapeo de píxeles.
- **Código generado por IA:** todos los archivos de esta carpeta (`regs.h`, `startup.c`, `stm32f446.ld`,
  `main.c`, `Makefile`, `README.md`) fueron generados por la IA en su versión inicial.
- **Revisión propia:** (completar) qué se verificó contra el RM0390 / datasheet, qué se modificó y por qué.

## Sesión 22-oct-2026 – diagnóstico del panel (Claude Code, modelo Claude Opus 5.5)

- **Qué hizo la IA, de forma autónoma** (Daniel ausente ~1 h): copió esta carpeta al repo, grabó
  `main.c`, y luego escribió y corrió un banco de pruebas, observando el panel con la webcam
  del portátil (ffmpeg) y leyendo la UART de la Nucleo.
- **Código generado por IA en esta sesión:**
  - `pruebas.c` (nuevo): variantes del protocolo HUB75 por bit-banging, retardos con DWT,
    autodiagnóstico de pines (pull-up/pull-down/salida en alto), control positivo de A/B y "fuzz" LFSR.
  - `regs.h`: se agregaron `DEMCR`, `DWT_CTRL`, `DWT_CYCCNT` (contador de ciclos).
  - `Makefile`: objetivos `pruebas` / `flash-pruebas` con parámetros `VAR`, `MIRROR`, `ROW`.
  - `PRUEBAS.md`: registro de cada prueba, resultado y conclusiones.
- **Fuera del repo** (no entregables): scripts auxiliares para grabar + tomar fotos.
- **Datos no verificados contra datasheet:** los comandos/registros de FM6126A e ICN2053/FM6353
  (número de CLK con LAT en alto, valores de registros) vienen de librerías abiertas; no hay
  datasheet oficial del chip del panel ("RR612…").
- **Revisión propia:** (completar) qué entendió/verificó Daniel del diagnóstico y del código.

## Sesión 22-oct-2026 (2.a parte) – verificación y mapeo (Claude Code, Claude Opus 5.5)

- Daniel corrigió el cableado (causa: filas del HUB75 invertidas). La IA corrió el autodiagnóstico,
  grabó `main.c` sin cambios y tomó/analizó fotos con la webcam.
- **Código generado por IA en esta parte:**
  - `pruebas.c`: `PINES_EN_CORTO` pasa a 0 (comentario con la historia); VAR 18 (MAPA, WALK lento
    sincronizado con UART) y VAR 19 (validación: dibuja marco y diagonales con la fórmula inversa).
  - `main.c`: cambio **temporal** a `MODE_FILL` para fotografiar FILL; se revirtió (archivo idéntico al original).
  - `MAPEO.md`, secciones 6-9 de `PRUEBAS.md`, actualización de `CLAUDE.md` y `README.md`.
- **Análisis de imágenes (fuera del repo):** scripts de Python + OpenCV para calibrar la cuadrícula del
  panel en las fotos (homografía) y ubicar el LED encendido. Con ellos se midieron las posiciones; la
  regla del mapeo la dedujo la IA de esas medidas y se comprobó en el panel con VAR 19.
- **Revisión propia:** (completar) Daniel debe poder derivar la fórmula de la tabla de MAPEO.md y
  explicar la fórmula inversa de VAR 19.

## Demo serpiente RGB (22-oct-2026, Claude Code)
- `serpiente.c` y sus objetivos `serpiente` / `flash-serpiente` del `Makefile` los generó la IA a pedido
  de Daniel: refresco por TIM3 copiado de `main.c`, `set_pixel()` con la fórmula de MAPEO.md, doble
  buffer con barrera de compilador, y recorrido en zig-zag. Probado en el panel (fotos/SERPIENTE_*.jpg).

## Diagnóstico panel congelado (26-oct-2026, Claude Code)
- La IA tomó fotos con la webcam (ffmpeg), leyó la UART, grabó `pruebas.c` VAR 0 (sin cambios) y
  dedujo de los 4 pines en corto que el conector HUB75 estaba girado 180° (PRUEBAS.md §11).
  No se modificó código.

## Serpiente simplificada estilo curso (26-oct-2026, Claude Code)
- A pedido de Daniel, la IA reescribió la serpiente siguiendo la estructura de los ejemplos del
  curso (repo divadnauj-GB/Comp-Arch-Emb-Sys, `5-ARM-Boards/NUCLEO-64/4-custom-hal` y `5-systick-delay`):
  `crt0.S` (arranque en ensamblador, basado en el del curso), `linker_script.ld`, `inc/stm32f446.h`,
  `src/gpio_config.c`, `src/SysTick.c`, `src/panel_hub75.c`, `main.c` y `Makefile`.
- Cambios de diseño para que sea fácil de explicar: sin interrupciones ni timers PWM (refresco por
  polling en el lazo principal), imagen `pantalla[fila][columna]` con un color por píxel, nombres
  completos y máscaras con nombre.
- Los archivos de pruebas se movieron, sin cambios, a `pruebas_y_diagnostico/`.
