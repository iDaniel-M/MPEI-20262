# Registro de pruebas – panel P10 RGB (22-oct-2026)

Situación inicial: con `main.c` el panel muestra 4 LEDs azules fijos en la última columna
(basura del arranque) y no cambia con nada de lo que manda la STM.

Método: firmware `pruebas.c`, una variante por compilación (`make pruebas VAR=n [MIRROR=1] [ROW=r]`,
`make flash-pruebas`). Todas las variantes intentan pintar **todo en rojo**; cualquier cambio en los
LEDs azules = algo entró al panel. Cada prueba: grabar, log de UART y 3 fotos con la webcam
(1 s entre fotos) en `fotos/` (`Vnn_Mm.jpg`: n = variante, m = 0 cable normal / 1 cable espejado).

## 1. Autodiagnóstico de pines (al arrancar, por UART)

Para cada pin: entrada con pull-down → leer; entrada con pull-up → leer; salida en ALTO 20 µs → leer IDR.

| Pin | Señal | PD | PU | OUT1 | Conclusión |
|---|---|---|---|---|---|
| PC0 | R1 | 0 | 1 | 1 | libre (entrada CMOS o cable suelto) |
| PC1 | G1 | 0 | 1 | 1 | libre |
| **PC2** | **B1** | 0 | **0** | **0** | **CORTO A GND** |
| PC3 | R2 | 0 | 1 | 1 | libre |
| PC4 | G2 | 0 | 1 | 1 | libre |
| **PC5** | **B2** | 0 | **0** | **0** | **CORTO A GND** |
| **PC6** | **OE** | 0 | **0** | **0** | **CORTO A GND** |
| PC7 | LAT | 0 | 1 | 1 | libre |
| PC8 | CLK | 0 | 1 | 1 | libre |
| PC10 | A | 0 | 1 | 1 | libre |
| PC11 | B | 0 | 1 | 1 | libre |

"OUT1 = 0" significa que aun manejando el pin en alto (push-pull, ~25-40 Ω) se lee 0 V: hay un corto
(una resistencia de más de ~100 Ω dejaría leer 1). **B1, B2 y OE están unidos a GND.**

Consecuencias:
- El OE del panel está siempre en 0 → siempre encendido. Por eso "OE parecía funcionar": no
  depende de la STM. El PWM de brillo de `main.c` en realidad estaba metiendo corriente de PC6 a GND.
- Desde aquí `pruebas.c` deja PC2, PC5 y PC6 como **entradas** (`PINES_EN_CORTO 1`) para no forzarlos.
- Coincidencia sospechosa: los pines HUB75 3, 7 y 15 tienen como vecinos (fila de enfrente del
  conector) a los pines 4, 8 y 16, que son GND. Eso sugirió un conector espejado → variantes `MIRROR=1`.

Intento extra, **inconcluso** (código retirado): medir con el ADC el voltaje con pull-up (el ADC no
lee en modo entrada, daba ~15 mV en todos) y medir capacitancia por tiempo de carga con DWT
(todos los pines dieron 35 ciclos; la resolución no alcanza).

## 2. Variantes de protocolo

Todas con PC2/PC5/PC6 protegidos (excepto la primera foto `V00_primera_sin_proteccion.jpg`).

| VAR | Qué cambia | Normal (M0) | Espejado (M1) |
|---|---|---|---|
| 0 | Base por GPIO: CLK rápido, LAT 1 µs, 64 bits | sin cambio | sin cambio |
| 1, 2 | OE fijo en 1 / en 0 | **no aplicable**: PC6 en corto | — |
| 3 | CLK lento 1 µs + 1 µs | sin cambio | sin cambio |
| 4 | CLK lento + LAT largo 20 µs | sin cambio | sin cambio |
| 5 | LAT activo en bajo | sin cambio | sin cambio |
| 6 | Datos invertidos | sin cambio | sin cambio |
| 7 | 32 bits por fase | sin cambio | sin cambio |
| 8 | 128 bits por fase | sin cambio | sin cambio |
| 9 | 256 bits por fase | sin cambio | sin cambio |
| 10 | Init FM6126A (REG1=7FFF, REG2=0040) | sin cambio | sin cambio |
| 11 | Captura en flanco de bajada | sin cambio | sin cambio |
| 12 | Salidas en drenador abierto | **cambia: queda 1 LED** (ver §3) | igual |
| 13 | Intento ICN2053/FM6353 (config + datos 16 bits + VSYNC + GCLK) | sin cambio* | sin cambio* |
| 14 | CLK MUY lento 10 kHz, LAT 100 µs | sin cambio | sin cambio |
| 16 | Solo A/B fijos en ROW=0..3 (control positivo) | **el LED cambia de fila** | — |
| 17 | FUZZ: LFSR en PC0,1,3,4,7,8 cada 1 µs | sin cambio | — |

\* En VAR 13 el GCLK sale por OE, y OE no se puede manejar (PC6 en corto / OE del panel a GND),
así que la prueba de drivers PWM **no es válida** hasta arreglar el cableado.

## 3. Lo que SÍ funcionó (control positivo)

- **VAR 12** (drenador abierto): A y B quedan sin manejar en alto → el barrido de filas se detiene y
  la columna de 4 LEDs pasa a 1 LED. Eso prueba que la columna de 4 era el mismo latch de basura
  mostrado en las 4 filas que barre A/B.
- **VAR 16** (`fotos/V16_filas_0123.jpg`): con A/B fijos el LED encendido cambia de fila:
  ROW=0 → fila 13, ROW=1 → fila 15, ROW=2 → fila 14, ROW=3 → fila 16 (contando desde arriba).
  **A (PC10) y B (PC11) llegan al panel y el panel reconoce sus niveles ALTOS de 3.3 V.**

## 4. Conclusiones

1. **No es (principalmente) un problema de nivel lógico.** A y B, manejados con 3.3 V, funcionan.
   En estos paneles A, B, CLK, LAT y datos suelen pasar por el mismo tipo de buffer (74HC245), así
   que un 74HCT245 probablemente no resolvería por sí solo.
2. **Hay un fallo de cableado/conexión:** B1, B2 y OE están en corto a GND, y CLK/LAT no producen
   efecto con ninguna temporización, polaridad, protocolo ni asignación de pines (incluido el fuzz,
   que habría funcionado si CLK y LAT estuvieran en cualquiera de PC0,1,3,4,7,8).
3. La hipótesis "conector espejado" explica el corto de los 3 pines, pero las pruebas con
   `MIRROR=1` tampoco cambiaron el panel, así que no está confirmada.

## 5. Qué revisar (hardware)

1. **Con el panel DESCONECTADO**, volver a grabar cualquier `pruebas.c` y mirar el diagnóstico UART:
   - si PC2/PC5/PC6 siguen en corto → el problema está del lado de la Nucleo/cables
     (cable tocando GND, puente de soldadura, o pin dañado);
   - si desaparece el corto → los pines 3, 7 y 15 del conector del panel están en GND: revisar
     orientación del conector (muesca/flecha, pin 1) o el pinout real del módulo.
2. Con un multímetro en continuidad: pin 15 (OE) del conector del panel contra GND (pin 16), y pin
   13/14 (CLK/LAT) del panel contra los pines PC8/PC7 de la Nucleo.
3. Si hay osciloscopio: ver CLK (pin 13) y LAT (pin 14) **en el conector del panel**, no en la Nucleo.
4. Una vez arreglado, poner `PINES_EN_CORTO 0` en `pruebas.c` y repetir VAR 0, 1, 2 y 13.
5. 74HCT245: dejarlo como mejora (márgenes de ruido con cables largos), no como causa probable.

**Ojo:** `main.c` maneja PC6 (OE, PWM) y B1/B2 en alto. Mientras exista el corto, no conviene
dejarlo corriendo: en la placa quedó grabado `pruebas.c` VAR 3, que no maneja esos pines.

---

# Segunda sesión (22-oct-2026, tarde) – cableado corregido

**Causa encontrada por Daniel:** las dos filas del conector HUB75 estaban invertidas (la hipótesis
"conector espejado" de la §1). B1, B2 y OE caían en los GND 4, 8 y 16. Se movió cada jumper a la otra
fila de su columna.

## 6. Autodiagnóstico después del arreglo (`pruebas.c`, VAR 16)

Los 11 pines dan `PD=0 PU=1 OUT1=1`: **ya no hay cortos**. Se puso `PINES_EN_CORTO 0` en `pruebas.c`
(los 11 pines vuelven a ser salidas).

¿Por qué `MIRROR=1` no funcionó si el cable sí estaba espejado? Hipótesis (no comprobada): con las filas
invertidas, el cable de GND de la Nucleo caía en el OE del panel, y los GND del panel (4, 8, 16) caían en
PC2, PC5 y PC6, que estaban como entradas. La Nucleo y el panel quedaban sin un GND común directo
(solo a través de los diodos de protección de esos pines), y las señales rápidas (CLK/LAT) no tenían
un retorno limpio. A y B son niveles lentos y estáticos, por eso sí pasaban.

## 7. `main.c` original (sin cambios), modo WALK

`make && make flash`. Fotos `fotos/WALK_<hora>.jpg` durante ~20 s y `fotos/WALK_mosaico.jpg`:
**el punto recorre el panel**. El panel por fin recibe datos.

## 8. Modo FILL

Versión temporal de `main.c` con `mode = MODE_FILL` al arrancar (sin botón). Fotos
`fotos/FILL_*.jpg` y `fotos/FILL_mosaico.jpg`: **pantalla completa en rojo, verde y azul**, uniforme
en las 16 filas y 32 columnas. Confirma que `BITS_PER_PANEL = 64` y `N_PHASES = 4` son correctos.
Luego se volvió a dejar `main.c` arrancando en WALK (idéntico al original, verificado con `cmp`).

## 9. Mapeo (fase, bit) → (x, y)

Resultado completo en **MAPEO.md**. Resumen: 4 bloques de 16 bits por fase (uno cada 8 columnas);
primeros 8 bits → fila `fase`, de derecha a izquierda; siguientes 8 → fila `fase+4`, de izquierda a
derecha; R2/G2/B2 = misma posición + 8 filas.

- VAR 18 (MAPA): WALK lento, 4 s por punto, sincronizado con la UART. Fotos en `fotos/mapa/`.
- VAR 19 (VALIDA): dibuja marco + 2 diagonales con la fórmula inversa → **sale perfecto**
  (`fotos/V19_validacion_2.jpg`).

## Estado final
- Hardware: OK. No hace falta el 74HCT245 (los 3.3 V funcionan; se puede agregar después si con
  cables largos o 2 paneles aparecen errores).
- En la placa quedó grabado `main.c` (modo WALK); luego se reemplazó por `serpiente.c` (§10).

## 10. Demo serpiente RGB
`serpiente.c` (`make serpiente && make flash-serpiente`): 24 LEDs con colores R, G, B alternados recorren
el panel en zig-zag a 25 pasos/s, brillo 36 % (se bajó desde 60 % porque deslumbraba). Funciona (`fotos/SERPIENTE_mosaico.jpg`). **Es lo que quedó grabado en la placa.**
