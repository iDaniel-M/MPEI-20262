/* Punto de entrada del programa bare-metal */
ENTRY(Reset_Handler)

/* Estructura de memoria del microcontrolador STM32F446RE */
MEMORY
{
  RAM    (xrw)    : ORIGIN = 0x20000000, LENGTH = 128K
  FLASH  (rx)     : ORIGIN = 0x08000000, LENGTH = 512K
}

/* Ubicación inicial del Stack Pointer al final de la RAM */
_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
  /* Tabla de vectores de interrupción en el inicio de la Flash */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >FLASH

  /* Código fuente y constantes de solo lectura */
  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text*)
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
    _etext = .;
  } >FLASH

  /* Variables globalmente inicializadas (.data) */
  _sidata = LOADADDR(.data);

  .data :
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data)
    *(.data*)
    . = ALIGN(4);
    _edata = .;
  } >RAM AT> FLASH

  /* Variables globales no inicializadas (.bss) */
  .bss :
  {
    . = ALIGN(4);
    _sbss = .;
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
    __bss_end__ = _ebss;
  } >RAM
}