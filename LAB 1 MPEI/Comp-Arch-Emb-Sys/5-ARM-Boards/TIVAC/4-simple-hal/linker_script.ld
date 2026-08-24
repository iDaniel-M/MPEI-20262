ENTRY(Reset_Handler)

MEMORY
{
    FLASH  (rx)  : ORIGIN = 0x00000000, LENGTH = 256K
    RAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 32K
}

SECTIONS
{
	.text : { *(.isr_vector);
			 *(.text*);
			} > FLASH

	.rodata  : { 
			*(.rodata*);
			*(.srodata*); 
			_sidata = .;
			}	> FLASH
			
	
	.data  : AT ( _sidata ) {  
				_sdata = .;
				*(.data*);
				*(.sdata*);
				_edata = .;
			} > RAM 

	.bss (NOLOAD) : { 
			_sbss = .;
			*(.bss*) 
			_ebss = .;
			}> RAM

	.stack (NOLOAD) :  {
		. = ORIGIN(RAM) + LENGTH(RAM);
		stack_top = .;
	} > RAM
}
