/*
 * linker_script.ld - Dónde va cada parte del programa en la memoria del STM32F446RE.
 * FLASH: 512 KB desde 0x08000000 (código y constantes, no se borra al apagar)
 * RAM:   128 KB desde 0x20000000 (variables y pila)
 */
ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 512K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

SECTIONS
{
    /* Código: primero la tabla de vectores (debe quedar en 0x08000000), luego el resto */
    .text : {
        *(.isr_vector);
        *(.text*);
    } > FLASH

    /* Constantes (const). Al final se marca _sidata: ahí empieza la copia de .data en FLASH */
    .rodata : {
        *(.rodata*);
        _sidata = .;
    } > FLASH

    /* Variables con valor inicial: viven en RAM, pero su valor inicial se guarda en
       FLASH en _sidata ("AT"). crt0.S las copia de _sidata a _sdata.._edata */
    .data : AT (_sidata) {
        _sdata = .;
        *(.data*);
        _edata = .;
    } > RAM

    /* Variables sin valor inicial: crt0.S pone en 0 de _sbss a _ebss */
    .bss (NOLOAD) : {
        _sbss = .;
        *(.bss*);
        *(COMMON);
        _ebss = .;
    } > RAM

    /* La pila empieza al final de la RAM y crece hacia abajo */
    .stack (NOLOAD) : {
        . = ORIGIN(RAM) + LENGTH(RAM);
        stack_top = .;
    } > RAM
}
