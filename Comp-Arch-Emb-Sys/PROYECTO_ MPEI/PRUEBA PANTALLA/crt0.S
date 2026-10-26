/*
 * crt0.S - Arranque del STM32F446RE en ensamblador (igual que los ejemplos del curso).
 *
 * Al encender, el Cortex-M4 lee de la dirección 0x08000000 (inicio de la FLASH):
 *   palabra 0 = valor inicial del puntero de pila (SP)
 *   palabra 1 = dirección de Reset_Handler (primera instrucción que se ejecuta)
 * Reset_Handler prepara la RAM para C y luego llama a main().
 */
.syntax unified     /* sintaxis moderna de ensamblador ARM */
.cpu cortex-m4      /* nuestro núcleo */
.thumb              /* el Cortex-M solo ejecuta instrucciones Thumb */

.global g_pfnVectors
.global Reset_Handler

/* _estack lo define linker_script.ld: es el final de la SRAM (la pila crece hacia abajo) */
.equ RAM_STACK_TOP, _estack

/* ===================== Tabla de vectores =====================
 * Solo se usan las excepciones del núcleo: este programa NO usa interrupciones
 * de periféricos (todo se hace por "polling" en el lazo de main). */
.section .isr_vector,"a",%progbits
g_pfnVectors:
    .word RAM_STACK_TOP       /* 0: puntero de pila inicial */
    .word Reset_Handler       /* 1: Reset */
    .word NMI_Handler         /* 2: NMI */
    .word HardFault_Handler   /* 3: Hard Fault */
    .word MemManage_Handler   /* 4: error de MPU */
    .word BusFault_Handler    /* 5: error de bus */
    .word UsageFault_Handler  /* 6: instrucción inválida */
    .word 0                   /* 7: reservado */
    .word 0                   /* 8: reservado */
    .word 0                   /* 9: reservado */
    .word 0                   /* 10: reservado */
    .word SVC_Handler         /* 11: SVCall */
    .word DebugMon_Handler    /* 12: Debug Monitor */
    .word 0                   /* 13: reservado */
    .word PendSV_Handler      /* 14: PendSV */
    .word SysTick_Handler     /* 15: SysTick (no se usa: el SysTick se lee por polling) */

/* ===================== Reset_Handler ===================== */
.section .text.Reset_Handler
.weak Reset_Handler
.type Reset_Handler, %function
Reset_Handler:
    /* 1) Copiar la sección .data (variables con valor inicial) de la FLASH a la RAM.
          R0 = destino en RAM, R1 = fin del destino, R2 = origen en FLASH */
    LDR R0, =_sdata
    LDR R1, =_edata
    LDR R2, =_sidata
    B LoopCopyDataInit

CopyDataInit:
    LDR R3, [R2], #4          /* R3 = palabra de la FLASH, y R2 avanza 4 bytes */
    STR R3, [R0], #4          /* guardar R3 en la RAM, y R0 avanza 4 bytes */

LoopCopyDataInit:
    CMP R0, R1                /* ¿ya llegamos al final de .data? */
    BLT CopyDataInit          /* si no, copiar otra palabra */

    /* 2) Poner en cero la sección .bss (variables sin valor inicial, C exige que valgan 0) */
    LDR R0, =_sbss
    LDR R1, =_ebss
    MOV R2, #0
    B LoopFillZerobss

FillZerobss:
    STR R2, [R0], #4          /* escribir 0 y avanzar 4 bytes */

LoopFillZerobss:
    CMP R0, R1                /* ¿ya llegamos al final de .bss? */
    BLT FillZerobss

    /* 3) Saltar al programa en C. (No llamamos __libc_init_array como en los
          ejemplos del curso porque este proyecto no usa la libc.) */
    BL main

_infinite_loop:               /* main nunca debería volver; si vuelve, quedarse aquí */
    B _infinite_loop

/* ===================== Excepciones no usadas =====================
 * "weak" = si algún archivo de C define una función con el mismo nombre, se usa esa.
 * Si no, todas caen aquí: un lazo infinito (B .) para poder verlo con el depurador. */
.weak NMI_Handler
.weak HardFault_Handler
.weak MemManage_Handler
.weak BusFault_Handler
.weak UsageFault_Handler
.weak SVC_Handler
.weak DebugMon_Handler
.weak PendSV_Handler
.weak SysTick_Handler

NMI_Handler:
HardFault_Handler:
MemManage_Handler:
BusFault_Handler:
UsageFault_Handler:
SVC_Handler:
DebugMon_Handler:
PendSV_Handler:
SysTick_Handler:
    B .
