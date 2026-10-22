/*
 * ARCHIVO LINKER SCRIPT (linker.ld)
 * Define la arquitectura física de memoria del microcontrolador y le indica 
 * al compilador en qué direcciones exactas ubicar el código y las variables.
 */

/* Le indica al procesador y al depurador que la función Reset_Handler es el punto de inicio absoluto */
ENTRY(Reset_Handler)

/* Definición de los bloques de memoria físicos basados en la hoja de datos (Datasheet) del STM32F446RE */
MEMORY
{
  /* Define la memoria Flash, empezando en la dirección 0x08000000, con tamaño de 512 Kilobytes, y permisos de lectura y ejecución (rx) */
  FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 512K 
  
  /* Define la memoria RAM, empezando en la dirección 0x20000000, con tamaño de 128 Kilobytes, y permisos de lectura, escritura y ejecución (xrw) */
  RAM (xrw)  : ORIGIN = 0x20000000, LENGTH = 128K 
}

/* 
 * Calcula y guarda la dirección tope de la pila (Stack Pointer). 
 * En procesadores ARM Cortex-M, la pila empieza en la dirección más alta de la RAM y crece hacia abajo. 
 */
_estack = ORIGIN(RAM) + LENGTH(RAM);

/* Mapeo de las secciones de tu código C a las memorias físicas definidas arriba */
SECTIONS
{
  /* Bloque exclusivo para la tabla de vectores de interrupción (g_pfnVectors) */
  .isr_vector :
  {
    /* Alinea la dirección actual en múltiplos de 4 bytes (32 bits) para evitar fallos de lectura en el bus ARM */
    . = ALIGN(4);               
    
    /* La directiva KEEP obliga al optimizador del compilador a no eliminar tu arreglo de vectores */
    KEEP(*(.isr_vector))        
    
    /* Realínea a 4 bytes tras terminar de asignar los vectores */
    . = ALIGN(4);               
  } >FLASH /* Asigna este bloque al inicio absoluto de la memoria FLASH (0x08000000) */

  /* Bloque para las instrucciones de código y variables constantes */
  .text :
  {
    /* Alinea el inicio del código a 4 bytes */
    . = ALIGN(4);               
    
    /* Extrae e incluye todo el código binario ejecutable (.text) proveniente de tus funciones en main.c */
    *(.text)                    
    
    /* Incluye cualquier código ejecutable empaquetado de subsecciones generadas por el compilador */
    *(.text*)                   
    
    /* Extrae e incluye los datos de solo lectura (.rodata) */
    *(.rodata)                  
    
    /* Incluye cualquier dato de solo lectura empaquetado por el compilador */
    *(.rodata*)                 
    
    /* Realínea a 4 bytes antes de cerrar la sección */
    . = ALIGN(4);               
  } >FLASH /* Asigna las instrucciones de manera permanente en la memoria FLASH */

  /* Bloque para variables globales no inicializadas explícitamente */
  .bss :
  {
    /* Alinea a 4 bytes el inicio de las variables RAM */
    . = ALIGN(4);               
    
    /* Crea la etiqueta dinámica '_sbss' (Start BSS) que guarda la dirección actual */
    _sbss = .;                  
    
    /* Incluye todas las variables globales no inicializadas o inicializadas en cero de tu archivo C */
    *(.bss)                     
    
    /* Incluye subsecciones adicionales de variables no inicializadas */
    *(.bss*)                    
    
    /* Agrupa variables globales comunes del estándar C que no encajan en otras jerarquías */
    *(COMMON)                   
    
    /* Realínea el contador a 4 bytes antes de finalizar el bloque RAM */
    . = ALIGN(4);               
    
    /* Crea la etiqueta dinámica '_ebss' (End BSS) */
    _ebss = .;                  
  } >RAM /* Asigna las variables a la memoria RAM */
}
