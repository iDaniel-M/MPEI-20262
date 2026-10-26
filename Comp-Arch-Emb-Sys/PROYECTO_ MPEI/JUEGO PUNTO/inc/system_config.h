#ifndef __SYSTEM_CONFIG_H__
#define __SYSTEM_CONFIG_H__
#include "stm32f446_regs.h"   // registros a mano (en vez de stm32f4xx.h de CMSIS)
#include "stdint.h"

/* POSITION_VAL(MASK) = posición del primer bit en 1 de la máscara.
   Ej: POSITION_VAL(0x3 << 10) = 10. CMSIS lo hace con instrucciones CLZ/RBIT;
   aquí se usa __builtin_ctz de GCC ("count trailing zeros" = ceros a la derecha). */
#define POSITION_VAL(MASK) (__builtin_ctz(MASK))

/* WRITE_REG_FIELD(REG, MASK, VAL): escribe VAL solo en los bits de MASK.
     1) REG & ~(MASK)               borra el campo
     2) (VAL << posición) & (MASK)  pone VAL en la posición del campo
   CORRECCIÓN respecto al profesor: él escribía "~MASK" sin paréntesis. Con
   MASK = GPIO_MODER_MODER0_Msk << (pin * 2), C calcula (~0x3) << (pin*2), que
   borra también los bits de TODOS los pines más bajos del puerto (se perdía la
   configuración de PB3 y PB4 al configurar PB5). Con "~(MASK)" solo se borra el campo. */
#define WRITE_REG_FIELD(REG, MASK, VAL) \
    ((REG) = ((REG) & ~(MASK)) | (((VAL) << POSITION_VAL(MASK)) & (MASK)))

/* READ_REG_FIELD(REG, MASK): lee solo los bits de MASK y los baja a la posición 0 */
#define READ_REG_FIELD(REG, MASK) \
    (((REG) & (MASK)) >> POSITION_VAL(MASK))


/*Declaration of two timers made by software updated every 1ms*/
typedef struct{
  unsigned int sw_tmr1_count;
  unsigned int sw_tmr2_count;
  unsigned int sw_tmr1_period;
  unsigned int sw_tmr2_period;
  unsigned int sw_tmr1_flag;
  unsigned int sw_tmr2_flag;
} SW_Timers;

void update_sw_timers(volatile SW_Timers* timer);

/* Reloj del sistema: clock_config() lo sube de 16 MHz (HSI) a 84 MHz con el PLL:
     HSI 16 MHz / M(16) = 1 MHz -> x N(336) = 336 MHz -> / P(4) = 84 MHz
   Buses: AHB = 84 MHz, APB1 = 84/2 = 42 MHz (máximo del APB1: 45 MHz), APB2 = 42 MHz.
   OJO TIM3: cuando el divisor de APB1 no es 1, el reloj de los timers del APB1 es
   el DOBLE de PCLK1: TIM3 recibe 2 x 42 = 84 MHz (RM0390, figura "Clock tree"). */
#define SYSCLK 84000000 
#define MICROSECONDS_PER_SECOND 1000000

#if SYSCLK == 84000000
    #define AHB1CLK SYSCLK
    #define APB1CLK (SYSCLK/2)
    #define APB2CLK APB1CLK
#else
    #define AHB1CLK SYSCLK
    #define APB1CLK SYSCLK
    #define APB2CLK APB1CLK
#endif


void clock_config(void);

void SysTick_Init(uint32_t us);
void SysTick_enable_IrQ(char val);
void SysTick_Delay_us(uint32_t delay);
void SysTick_Delay_ms(uint32_t delay);


#endif /* __SYSTEM_CONFIG_H__ */