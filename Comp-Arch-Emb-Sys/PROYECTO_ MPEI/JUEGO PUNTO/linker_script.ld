/*
 * linker_script.ld - el del ejercicio 21-SPI-4 del profesor.
 * Único cambio: RAM = 128K (el STM32F446RE tiene 128 KB; el F401 del profesor, 96 KB).
 */
ENTRY(Reset_Handler)                     /* primera instrucción: Reset_Handler de crt0.S */

MEMORY
{
    FLASH  (rx)  : ORIGIN = 0x08000000, LENGTH = 512K   /* programa y constantes */
    RAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 128K     /* variables y pila */
}

SECTIONS
{
	/* Tabla de vectores primero (el micro la lee en 0x08000000) y luego el código */
	.text : { *(.isr_vector);
			 *(.text*);
			} > FLASH

	/* Constantes y lo que necesita la libc (sprintf): .init/.fini, .ARM.exidx */
	.rodata  : {
			*(.rodata*);
			*(.srodata*);
			. = ALIGN(4);
			KEEP(*(.init))
			KEEP(*(.fini))
			*(.eh_frame)
			*(.ARM.exidx)
    		_etext = .;

			}	> FLASH

			/* dirección en FLASH de los valores iniciales de .data (crt0.S los copia) */
			_sidata = LOADADDR(.data);

	/* Variables con valor inicial: viven en RAM, su valor inicial se guarda en FLASH */
	.data  : {
				_sdata = .;
				*(.data*);
				*(.sdata*);
				KEEP(*(.init_array))
    			KEEP(*(.fini_array))
				_edata = .;
			} > RAM AT> FLASH

	/* Variables sin valor inicial: crt0.S las pone en 0 */
	.bss (NOLOAD) : {
			_sbss = .;
			__bss_start__ = _sbss;
			*(.bss*)
			*(COMMON)
			_ebss = .;
			__bss_end__ = _ebss;
			}> RAM

	/* La pila empieza al final de la RAM y crece hacia abajo */
	.stack (NOLOAD) :  {
		. = ORIGIN(RAM) + LENGTH(RAM);
		stack_top = .;
		_estack = stack_top;
	} > RAM
}
