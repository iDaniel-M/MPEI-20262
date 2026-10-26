#include "SysTick.h"

#define SYSCLK 16000000U                 // reloj del micro: HSI de 16 MHz
#define TICKS_POR_MICROSEGUNDO (SYSCLK / 1000000U)   // 16 cuentas = 1 us

/*
 * Espera "microsegundos" usando el SysTick por polling (sin interrupción).
 * El contador empieza en STRVR, baja 1 cada ciclo de reloj y, al llegar a 0,
 * pone COUNTFLAG en 1. Máximo: 2^24 / 16 = 1 048 575 us (~1 s).
 */
void SysTick_Delay_us(uint32_t microsegundos)
{
    SysTick->STCSR = 0;                                        // 1) apagar el contador
    SysTick->STRVR = microsegundos * TICKS_POR_MICROSEGUNDO - 1; // 2) contar N cuentas: de N-1 a 0
    SysTick->STCVR = 0;                                        // 3) escribir STCVR lo pone en 0 y borra COUNTFLAG
    SysTick->STCSR = SYSTICK_STCSR_ENABLE | SYSTICK_STCSR_CLKSOURCE; // 4) arrancar con el reloj de 16 MHz

    while ((SysTick->STCSR & SYSTICK_STCSR_COUNTFLAG) == 0) {  // 5) esperar a que llegue a 0
    }

    SysTick->STCSR = 0;                                        // 6) apagar el contador
}
