/*
 * stm32f446.ld - Linker script para STM32F446RE
 *   FLASH: 512 KB desde 0x0800 0000
 *   SRAM : 128 KB desde 0x2000 0000 (SRAM1 112 KB + SRAM2 16 KB, contiguas)
 */

ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 512K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

/* El stack crece hacia abajo desde el final de la RAM */
_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
    /* Tabla de vectores: DEBE ir en 0x0800 0000 */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))   /* KEEP: que --gc-sections no la elimine */
        . = ALIGN(4);
    } > FLASH

    /* Código y constantes */
    .text :
    {
        . = ALIGN(4);
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    /* Dirección en FLASH donde queda guardada la copia inicial de .data */
    _sidata = LOADADDR(.data);

    /* Variables globales inicializadas: viven en RAM (VMA) pero sus valores
       iniciales se guardan en FLASH (LMA). Reset_Handler las copia. */
    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT > FLASH

    /* Variables globales sin inicializar: Reset_Handler las pone en 0 */
    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    /* Comprobación: dejar al menos 4 KB libres para el stack */
    ASSERT(_ebss + 4K <= _estack, "No queda RAM suficiente para el stack")
}
