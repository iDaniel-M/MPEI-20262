/*
 * startup.c - Arranque bare metal del STM32F446RE.
 *
 * Al salir de reset, el Cortex-M4 lee de la dirección 0x0800 0000:
 *   palabra 0 -> valor inicial del Stack Pointer (MSP)
 *   palabra 1 -> dirección de Reset_Handler
 * y luego salta a Reset_Handler. Aquí no existe nada más: no hay C runtime,
 * así que este archivo debe preparar la RAM antes de llamar a main().
 */
#include <stdint.h>
#include "regs.h"

/* Símbolos definidos en el linker script (stm32f446.ld) */
extern uint32_t _estack;   /* fin de la RAM = tope del stack        */
extern uint32_t _sidata;   /* donde está la copia de .data en FLASH */
extern uint32_t _sdata, _edata;  /* inicio/fin de .data en RAM     */
extern uint32_t _sbss,  _ebss;   /* inicio/fin de .bss  en RAM     */

int main(void);
void Reset_Handler(void);
void Default_Handler(void);
void TIM3_IRQHandler(void);   /* definido en main.c */

/* Cualquier interrupción no esperada cae aquí y se queda en un lazo:
   así se nota con el debugger en vez de saltar a una dirección basura. */
void Default_Handler(void)
{
    while (1) { }
}

/* Tabla de vectores: 16 excepciones del núcleo + 97 IRQ del F446.
   La sección ".isr_vector" el linker la coloca de primera en FLASH. */
#define N_VECTORS (16 + 97)

__attribute__((section(".isr_vector"), used))
void (* const vector_table[N_VECTORS])(void) = {
    [0]  = (void (*)(void))&_estack,  /* MSP inicial                    */
    [1]  = Reset_Handler,             /* Reset                          */
    [2]  = Default_Handler,           /* NMI                            */
    [3]  = Default_Handler,           /* HardFault                      */
    [4]  = Default_Handler,           /* MemManage                      */
    [5]  = Default_Handler,           /* BusFault                       */
    [6]  = Default_Handler,           /* UsageFault                     */
    [11] = Default_Handler,           /* SVCall                         */
    [14] = Default_Handler,           /* PendSV                         */
    [15] = Default_Handler,           /* SysTick                        */
    [16 + TIM3_IRQn] = TIM3_IRQHandler, /* IRQ 29: TIM3 -> refresco HUB75 */
};

void Reset_Handler(void)
{
    /* 1) Copiar .data (variables globales con valor inicial) de FLASH a RAM */
    uint32_t *src = &_sidata;
    for (uint32_t *dst = &_sdata; dst < &_edata; )
        *dst++ = *src++;

    /* 2) Poner en cero .bss (variables globales sin valor inicial) */
    for (uint32_t *dst = &_sbss; dst < &_ebss; )
        *dst++ = 0;

    /* 3) Habilitar la FPU (CP10 y CP11 con acceso completo). Se compila con
          -mfloat-abi=hard: si el compilador llegara a usar una instrucción
          de punto flotante con la FPU apagada habría un UsageFault. */
    SCB_CPACR |= (0xFu << 20);

    main();

    while (1) { }   /* main no debería retornar */
}
