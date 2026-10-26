/* ==========================================================
 * linker_script.ld - Linker script para STM32F446RE (Nucleo-64)
 * Flash: 512 KB @ 0x08000000  |  SRAM: 128 KB @ 0x20000000
 *
 * El linker junta los .o y decide en qué dirección queda cada cosa.
 * Este archivo le dice qué memorias hay y en qué orden poner las secciones.
 * ========================================================== */

/* Primera instrucción del programa (la usa el depurador; el micro arranca
   leyendo la tabla de vectores, que apunta a Reset_Handler de crt0.S) */
ENTRY(Reset_Handler)

/* Memorias del micro: nombre, permisos (r = leer, w = escribir, x = ejecutar),
   dirección donde empiezan y tamaño. Datos del manual RM0390 */
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 512K   /* programa, no se borra al apagar */
    SRAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 128K   /* variables, se borra al apagar */
}

/* La pila empieza al FINAL de la SRAM (0x20020000) y crece hacia abajo.
   crt0.S pone este valor en la palabra 0 de la tabla de vectores */
_estack = ORIGIN(SRAM) + LENGTH(SRAM);

SECTIONS
{
    /* 1) Tabla de vectores: va PRIMERO en la FLASH (0x08000000), porque ahí la
          busca el micro al arrancar. KEEP = no borrarla aunque nadie la llame */
    .isr_vector :
    {
        KEEP(*(.isr_vector))
    } > FLASH

    /* 2) Código (.text) y constantes (.rodata), también en FLASH.
          ALIGN(4) = avanzar hasta una dirección múltiplo de 4 bytes */
    .text :
    {
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    /* 3) Variables CON valor inicial (ej: int x = 5;).
          "> SRAM"     : el programa las usa en la SRAM
          "AT > FLASH" : pero su valor inicial se guarda en la FLASH
          crt0.S copia los valores de la FLASH a la SRAM al arrancar.
          _sdata y _edata = dónde empieza y termina .data en la SRAM */
    .data :
    {
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > SRAM AT > FLASH

    /* Dirección en la FLASH donde quedó guardada la copia de .data (origen de la copia) */
    _sidata = LOADADDR(.data);

    /* 4) Variables SIN valor inicial (ej: int y;). C exige que valgan 0, así que
          crt0.S pone en 0 todo lo que hay entre _sbss y _ebss.
          COMMON = variables globales sin inicializar de compiladores viejos */
    .bss :
    {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > SRAM
}
