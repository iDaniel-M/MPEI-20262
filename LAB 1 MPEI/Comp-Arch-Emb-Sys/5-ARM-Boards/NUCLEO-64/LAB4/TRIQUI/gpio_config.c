#include "gpio_config.h"

void configurar_pines_gpio(void)
{
    // 1. Habilitar el reloj para los puertos GPIOA, GPIOB y GPIOC simultáneamente (optimizando las líneas originales)
    // El 7 (binario 0111) prende directamente los 3 primeros bits del bus AHB1
    RCC->AHB1ENR |= 7; 

    // Lectura pasiva para permitir que la señal de reloj se propague en el silicio antes de usar los GPIO
    volatile unsigned int retardo = RCC->AHB1ENR;
    (void)retardo;

    // 2. ENTRADAS: Configurar los pines de filas (salidas) del teclado matricial en GPIOB
    uint8_t pines_filas[4] = {1, 0, 12, 15};
    for(uint8_t idx = 0; idx < 4; idx++) {
        uint8_t pin_actual = pines_filas[idx];
        // Resetea los dos bits correspondientes a la configuración de modo de este pin
        GPIOB->MODER &= ~(3 << (pin_actual * 2));
        // Escribe un '01' lógico para declararlo como salida de propósito general
        GPIOB->MODER |=  (1 << (pin_actual * 2));
    }

    // Configurar los pines de columnas (entradas) del teclado matricial en GPIOB
    uint8_t pines_columnas[4] = {5, 6, 7, 8};
    for(uint8_t idx = 0; idx < 4; idx++) {
        uint8_t pin_actual = pines_columnas[idx];
        // Resetea a modo entrada
        GPIOB->MODER &= ~(3 << (pin_actual * 2));
        // Resetea las configuraciones previas de resistencia de pull
        GPIOB->PUPDR &= ~(3 << (pin_actual * 2));
        // Escribe un '01' para activar la resistencia interna Pull-Up (mantendrá en ALTO lógico por defecto)
        GPIOB->PUPDR |=  (1 << (pin_actual * 2));
    }

    // 3. SALIDAS: Configuración masiva de LEDs en el GPIOA
    // Uso de un arreglo const (inmutable) que iterará sobre las 14 salidas asociadas
    const uint8_t salidas_gpioa[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 15};
    for(uint8_t idx = 0; idx < 14; idx++) {
        uint8_t pin_actual = salidas_gpioa[idx];
        // Reseteo del modo actual del registro
        GPIOA->MODER &= ~(3 << (pin_actual * 2));
        // Configuración directa a salida general ('01')
        GPIOA->MODER |=  (1 << (pin_actual * 2)); 
    }

    // Configuración aislada del único LED de la máquina conectado en GPIOB (Pin 9)
    GPIOB->MODER &= ~(3 << (9 * 2));
    GPIOB->MODER |=  (1 << (9 * 2));

    // Configuración masiva de los 3 LEDs asociados a GPIOC
    const uint8_t salidas_gpioc[] = {13, 14, 15};
    for(uint8_t idx = 0; idx < 3; idx++) {
        uint8_t pin_actual = salidas_gpioc[idx];
        GPIOC->MODER &= ~(3 << (pin_actual * 2));
        GPIOC->MODER |=  (1 << (pin_actual * 2));
    }
}

// Abstracción para leer si un pin particular está tirado a tierra o recibiendo 3.3v
uint8_t leer_estado_pin(volatile GPIO_TypeDef *puerto, uint8_t pin) {
    // Comprobamos el bit de IDR con una máscara de corrimiento. Si el resultado es distinto de 0, devuelve 1.
    if (puerto->IDR & (1 << pin)) {
        return 1;
    }
    return 0; // Si el bit es 0 (tecla presionada debido al Pull-Up invertido), devuelve 0
}

// Abstracción para inyectar un estado en los diodos emisores de luz
void escribir_estado_pin(volatile GPIO_TypeDef *puerto, uint8_t pin, uint8_t estado) {
    if(estado) {
        // Enciende: Fuerza un 1 lógico en la posición del pin dentro del registro ODR
        puerto->ODR |= (1 << pin);
    } else {
        // Apaga: Fuerza un 0 lógico usando la compuerta AND y una máscara negada
        puerto->ODR &= ~(1 << pin);
    }
}