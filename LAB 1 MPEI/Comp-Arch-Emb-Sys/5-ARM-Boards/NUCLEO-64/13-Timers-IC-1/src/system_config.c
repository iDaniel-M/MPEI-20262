#include "system_config.h" // Incluye las definiciones del sistema (declaración de la estructura SW_Timers, macros de registros y constantes de reloj)

// Función que actualiza el estado de los temporizadores por software gestionados mediante SysTick
void update_sw_timers(volatile SW_Timers* timer){
  timer->sw_tmr1_count++; // Incrementa el contador interno del temporizador por software 1
  timer->sw_tmr2_count++; // Incrementa el contador interno del temporizador por software 2
  
  // Evalúa si el contador del temporizador 1 ha alcanzado su período configurado
  if(timer->sw_tmr1_count==timer->sw_tmr1_period){
    timer->sw_tmr1_count=0; // Reinicia el contador del temporizador 1 a cero
    timer->sw_tmr1_flag=1;  // Activa la bandera (flag) que indica el vencimiento del temporizador 1
  }
  
  // Evalúa si el contador del temporizador 2 ha alcanzado su período configurado
  if(timer->sw_tmr2_count==timer->sw_tmr2_period){
    timer->sw_tmr2_count=0; // Reinicia el contador del temporizador 2 a cero
    timer->sw_tmr2_flag=1;  // Activa la bandera (flag) que indica el vencimiento del temporizador 2
  }
}

// Inicializa el temporizador SysTick para desbordar a un intervalo específico medido en microsegundos (us)
void SysTick_Init(uint32_t us){
    uint32_t period; // Variable local para almacenar el valor calculado que se cargará en el registro RELOAD
    
    // Calcula la cantidad de ciclos de reloj requeridos para la demora en microsegundos y descuenta 1 por el estado 0
    period = (SYSCLK / MICROSECONDS_PER_SECOND) * us - 1; 
    
    WRITE_REG_FIELD(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk, 0); // Deshabilita temporalmente el conteo del SysTick para configurarlo
    WRITE_REG_FIELD(SysTick->LOAD, SysTick_LOAD_RELOAD_Msk, period); // Escribe el valor de recarga (RELOAD) calculado en el registro de 24 bits
    WRITE_REG_FIELD(SysTick->VAL, SysTick_VAL_CURRENT_Msk, 0); // Limpia el valor actual del contador (CURRENT) y borra la bandera COUNTFLAG
    WRITE_REG_FIELD(SysTick->CTRL, SysTick_CTRL_CLKSOURCE_Msk, 1); // Selecciona el reloj del procesador (AHB sin división) como fuente de conteo
    WRITE_REG_FIELD(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk, 1); // Activa el contador del SysTick con la nueva configuración
    WRITE_REG_FIELD(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN_Msk, 1); // Habilita el reloj para el periférico de configuración del sistema (SYSCFG) en el bus APB2
}

// Controla la activación de la interrupción del temporizador SysTick
void SysTick_enable_IrQ(char val){
    WRITE_REG_FIELD(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk, val); // Habilita (1) o deshabilita (0) la generación de interrupciones por desbordamiento del SysTick
}

// Genera un retardo bloqueante en microsegundos utilizando la bandera de desbordamiento (COUNTFLAG) del SysTick
void SysTick_Delay_us(uint32_t delay){
    for (uint32_t i=0; i<delay; i++) {
     // Permanece en bucle de espera hasta que la bandera COUNTFLAG se ponga en 1 (indicando que el contador llegó a 0)
     while(!(READ_REG_FIELD(SysTick->CTRL, SysTick_CTRL_COUNTFLAG_Msk))); 
    }    
}

// Genera un retardo bloqueante en milisegundos mediante la llamada sucesiva del retardo en microsegundos
void SysTick_Delay_ms(uint32_t delay){
    for (uint32_t i=0; i<delay; i++) {
     SysTick_Delay_us(1000); // Llama a la función de retardo por microsegundos pasando un intervalo de 1000 us (1 ms)
    }
}

// Configura el árbol de relojes del microcontrolador (RCC) para operar con el PLL a una frecuencia del sistema de 84 MHz
void clock_config(void){

    #if SYSCLK == 84000000 // Condicional de compilación: se ejecuta solo si la constante SYSCLK está configurada a 84 MHz

    // Ajusta los ciclos de espera (wait states) de la memoria Flash a 3 WS para prevenir errores de lectura a 84 MHz
    WRITE_REG_FIELD(FLASH->ACR,FLASH_ACR_LATENCY,FLASH_ACR_LATENCY_3WS); 

    WRITE_REG_FIELD(RCC->PLLCFGR,RCC_PLLCFGR_PLLM,16);  // Configura el factor de división de entrada del PLL (PLLM = 16)
    WRITE_REG_FIELD(RCC->PLLCFGR,RCC_PLLCFGR_PLLN,336); // Configura el factor de multiplicación del VCO del PLL (PLLN = 336)
    WRITE_REG_FIELD(RCC->PLLCFGR,RCC_PLLCFGR_PLLP,1);   // Configura el factor de división para la salida principal del sistema (PLLP = 4, valor de registro 1 representa división por 4)
    WRITE_REG_FIELD(RCC->PLLCFGR,RCC_PLLCFGR_PLLQ,7);   // Configura el factor de división para periféricos como USB/SDIO (PLLQ = 7)

    WRITE_REG_FIELD(RCC->CFGR,RCC_CFGR_PPRE1,4); // Asigna un preescalador de 2 para el bus APB1 (84 MHz / 2 = 42 MHz, frecuencia máxima del APB1)
    WRITE_REG_FIELD(RCC->CFGR,RCC_CFGR_PPRE2,0); // Configura sin división el bus APB2 (PPRE2 = 0 representa división por 1, opera a 84 MHz)
    WRITE_REG_FIELD(RCC->CFGR,RCC_CFGR_HPRE,0);  // Configura sin división el bus de alta velocidad AHB (HPRE = 0 representa división por 1, opera a 84 MHz)

    WRITE_REG_FIELD(RCC->CR,RCC_CR_PLLON,1); // Enciende el módulo del circuito de lazo de seguimiento de fase (PLL)
    while (! (READ_REG_FIELD(RCC->CR,RCC_CR_PLLRDY))); // Bloquea la ejecución hasta que la bandera PLLRDY confirme que el PLL está bloqueado y estable

    WRITE_REG_FIELD(RCC->CFGR,RCC_CFGR_SW,RCC_CFGR_SW_PLL); // Selecciona la salida del PLL como la fuente principal del reloj del sistema (SYSCLK)
    //RCC->CFGR |= (RCC_CFGR_SW_PLL << RCC_CFGR_SW_Pos); // Línea alternativa mediante operaciones a nivel de bits (comentada)
    while (!(READ_REG_FIELD(RCC->CFGR,RCC_CFGR_SWS_PLL))); // Espera a que el hardware confirme el cambio efectivo del reloj a la fuente PLL
    
    #else // Rama vacía en caso de que SYSCLK sea diferente de 84 MHz
    #endif
   
}