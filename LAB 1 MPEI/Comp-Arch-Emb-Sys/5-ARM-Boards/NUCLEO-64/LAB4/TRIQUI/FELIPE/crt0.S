.syntax unified
.cpu cortex-m4
.thumb

.global g_pfnVectors
.global Reset_Handler
.global _infinite_loop

/* 1. Tabla de Vectores obligatoria al inicio de la Flash (0x08000000) */
.section .isr_vector, "a", %progbits
g_pfnVectors:
    .word _estack            /* 0x00: Puntero de pila inicial */
    .word Reset_Handler      /* 0x04: Reset Handler */

/* 2. Código de Inicio (Bootstrapping) */
.section .text.Reset_Handler
.type Reset_Handler, %function
Reset_Handler:
    /* Inicializar el puntero de pila explícitamente */
    LDR R0, =_estack
    MOV SP, R0

    /* Copiar segmento .data de Flash a RAM */
    LDR R0, =_sdata
    LDR R1, =_edata
    LDR R2, =_sidata
    B LoopCopyDataInit

CopyDataInit:
    LDR R3, [R2], #4
    STR R3, [R0], #4

LoopCopyDataInit:
    CMP R0, R1
    BLT CopyDataInit

    /* Rellenar con ceros el segmento .bss */
    LDR R0, =_sbss
    LDR R1, =_ebss
    MOV R2, #0
    B LoopFillZerobss

FillZerobss:
    STR R2, [R0], #4

LoopFillZerobss:
    CMP R0, R1
    BLT FillZerobss

    /* Llamar a la función principal main() */
    BL main

.type _infinite_loop, %function
_infinite_loop:
    B _infinite_loop