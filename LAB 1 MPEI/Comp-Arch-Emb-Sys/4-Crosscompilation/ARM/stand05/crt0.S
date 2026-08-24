.syntax unified
.cpu cortex-m4
.thumb

.global g_pfnVectors
.global Reset_Handler

/* Default empty handler */
.equ RAM_STACK_TOP, stack_top /* Adjust based on SRAM size (128KB for STM32F405) */

.section .isr_vector,"a",%progbits
g_pfnVectors:
    .word RAM_STACK_TOP       /* Initial Stack Pointer */
    .word Reset_Handler       /* Reset Handler */
    .word NMI_Handler
    .word HardFault_Handler
    .word MemManage_Handler
    .word BusFault_Handler
    .word UsageFault_Handler
    .word 0                   /* Reserved */
    .word 0                   /* Reserved */
    .word 0                   /* Reserved */
    .word 0                   /* Reserved */
    .word SVC_Handler
    .word DebugMon_Handler
    .word 0                   /* Reserved */
    .word PendSV_Handler
    .word SysTick_Handler

.section .text.Reset_Handler
.weak Reset_Handler
.type Reset_Handler, %function
Reset_Handler:
    /* Copy data segment from flash to RAM */
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

    

    /* Zero fill the bss segment */
    
    LDR R0, =_sbss
    LDR R1, =_ebss
    MOV R2, #0
    B LoopFillZerobss

FillZerobss:
    STR R2, [R0], #4

LoopFillZerobss:
ToFillZerobss:
    CMP R0, R1
    BLT FillZerobss

    BL main
    
_infinite_loop:
    B _infinite_loop

/* Provide weak aliases for unhandled exceptions */
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


