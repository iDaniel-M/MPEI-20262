#include "gpio_config.h"

void configurar_pines_gpio(void)
{
    // 1. Habilitar el reloj para los puertos GPIOA, GPIOB y GPIOC
    // Se define un puntero volatil apuntando a la direccion de memoria absoluta del registro AHB1ENR.
    // La direccion se compone de: Base de perifericos (0x40000000) + Offset AHB1 (0x20000) + Offset RCC (0x3800) + Offset AHB1ENR (0x30) = 0x40023830.
    volatile uint32_t *puntero_reloj = (volatile uint32_t *)(0x40023830);

    // Se inyecta un 1 logico en los bits 0, 1 y 2 correspondientes a los puertos A, B y C.
    // El operador OR (|) garantiza que se sumen estos bits sin apagar otros relojes del sistema que ya esten activos.
    *puntero_reloj |= (1 << 0) | (1 << 1) | (1 << 2); 

    // Lectura pasiva del registro para forzar al procesador a esperar 
    // que la señal electrica del reloj se estabilice en los perifericos antes de proceder a configurarlos.
    volatile uint32_t retardo = *puntero_reloj;
    (void)retardo;

    // 2. ENTRADAS: Configurar los pines de filas (salidas logicas) del teclado matricial en GPIOB
    // Arreglo con los numeros de pin fisicos para iterar secuencialmente y optimizar la escritura.
    uint8_t pines_filas[4] = {1, 0, 12, 15};
    for(uint8_t idx = 0; idx < 4; idx++) {
        uint8_t pin_actual = pines_filas[idx];
        // Mascara AND negada: Fuerza un '00' en los dos bits de configuracion de modo del pin actual.
        GPIOB->MODER &= ~(3 << (pin_actual * 2));
        // Mascara OR: Escribe '01' en esos mismos bits para establecer el pin como Salida de Proposito General.
        GPIOB->MODER |=  (1 << (pin_actual * 2));
    }

    // Configurar los pines de columnas (entradas logicas) del teclado matricial en GPIOB
    uint8_t pines_columnas[4] = {5, 6, 7, 8};
    for(uint8_t idx = 0; idx < 4; idx++) {
        uint8_t pin_actual = pines_columnas[idx];
        // Mascara AND negada: Establece '00' para configurar el pin estrictamente como Entrada.
        GPIOB->MODER &= ~(3 << (pin_actual * 2));
        // Limpia (pone en '00') la configuracion de resistencia de anclaje (Pull-Up/Pull-Down).
        GPIOB->PUPDR &= ~(3 << (pin_actual * 2));
        // Escribe '01' para habilitar la resistencia Pull-Up interna. 
        // Esto asegura que el pin lea un 1 logico (3.3V) mientras el boton no sea presionado.
        GPIOB->PUPDR |=  (1 << (pin_actual * 2));
    }

    // 3. SALIDAS: Configuracion masiva de LEDs en el GPIOA
    // Arreglo inmutable con todos los pines del puerto A asignados a LEDs.
    const uint8_t salidas_gpioa[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 15};
    for(uint8_t idx = 0; idx < 14; idx++) {
        uint8_t pin_actual = salidas_gpioa[idx];
        // Limpia el modo actual del pin.
        GPIOA->MODER &= ~(3 << (pin_actual * 2));
        // Configura el pin como salida general inyectando '01'.
        GPIOA->MODER |=  (1 << (pin_actual * 2)); 
    }

    // Configuracion aislada del LED conectado al puerto GPIOB (Pin 9)
    // El desplazamiento es de 18 posiciones (9 * 2) porque cada pin ocupa 2 bits contiguos en el registro.
    GPIOB->MODER &= ~(3 << (9 * 2));
    GPIOB->MODER |=  (1 << (9 * 2));

    // Configuracion masiva de los 3 LEDs asignados al puerto GPIOC
    const uint8_t salidas_gpioc[] = {13, 14, 15};
    for(uint8_t idx = 0; idx < 3; idx++) {
        uint8_t pin_actual = salidas_gpioc[idx];
        // Limpia y establece cada pin como salida ('01').
        GPIOC->MODER &= ~(3 << (pin_actual * 2));
        GPIOC->MODER |=  (1 << (pin_actual * 2));
    }
}

// Funcion que evalua si un pin de entrada esta conectado a tierra (boton presionado) o recibiendo voltaje.
uint8_t leer_estado_pin(volatile GPIO_TypeDef *puerto, uint8_t pin) {
    // Se aisla el bit del pin especifico dentro del registro de entrada de datos (IDR) usando una compuerta AND.
    // Si el bit es 1 (voltaje presente por el Pull-Up), la condicion es verdadera.
    if (puerto->IDR & (1 << pin)) {
        return 1;
    }
    // Si el bit es 0 (el boton derivo el voltaje a tierra), retorna 0 indicando presion.
    return 0; 
}

// Funcion que altera el estado electrico de un pin configurado como salida.
void escribir_estado_pin(volatile GPIO_TypeDef *puerto, uint8_t pin, uint8_t estado) {
    if(estado) {
        // Operacion Set: Inyecta un 1 logico usando OR, activando la salida sin afectar el resto del registro ODR.
        puerto->ODR |= (1 << pin);
    } else {
        // Operacion Reset: Inyecta un 0 logico usando AND con una mascara negada, cortando el voltaje del pin especifico.
        puerto->ODR &= ~(1 << pin);
    }
}