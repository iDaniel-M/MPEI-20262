#include <stdint.h>
#include "stm32f401.h"
#include "game.h"
#include "gpio_config.h"

// Rediseñamos la estructura para que los nombres sean más claros en español
typedef struct {
    GPIO_TypeDef *puerto_j1; // Registro base del puerto para el Jugador 1
    uint8_t pin_j1;          // Número de pin exacto del Jugador 1
    
    GPIO_TypeDef *puerto_j2; // Registro base del puerto para el Bot
    uint8_t pin_j2;          // Número de pin exacto del Bot
} MapeoCasillas;

// Array estructural constante que vincula la lógica estricta de hardware de los 18 pines a las 9 casillas lógicas
const MapeoCasillas mapa_leds[9] = {
    { GPIOA, 2,  GPIOA, 6  }, // Celda superior izquierda
    { GPIOA, 15, GPIOA, 10 }, // Celda superior central
    { GPIOC, 13, GPIOA, 1  }, // Celda superior derecha
    { GPIOA, 3,  GPIOA, 5  }, // Celda media izquierda
    { GPIOA, 11, GPIOA, 8  }, // Celda centro
    { GPIOC, 15, GPIOA, 0  }, // Celda media derecha
    { GPIOA, 4,  GPIOA, 7  }, // Celda inferior izquierda
    { GPIOA, 12, GPIOA, 9  }, // Celda inferior central
    { GPIOC, 14, GPIOB, 9  }  // Celda inferior derecha
};

// Implementación de retardo crudo que gasta ciclos de reloj quemándolos en un bucle while (cambiado del for original)
void pausa_milisegundos(volatile uint32_t ms) {
    // El procesador a 16MHz estándar quema aprox 4 ciclos por instrucción. Esto empareja la temporalidad.
    uint32_t ciclos = ms * 4000; 
    while (ciclos > 0) {
        __asm("nop"); // Ensamblador puro (No-Operation) que instruye a la CPU a hacer nada un ciclo
        ciclos--;
    }
}

// Función encargada del multiplexado de las teclas
char escanear_teclado_matricial(void) {
    // Distribución física del teclado membrana
    const char matriz_teclas[4][4] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
    };

    uint8_t filas[4] = {1, 0, 12, 15};
    uint8_t columnas[4] = {5, 6, 7, 8};

    // Escaneo de filas por multiplexación
    for (uint8_t f = 0; f < 4; f++) {
        // Establecemos en alto (1) todas las filas para anular lecturas falsas
        for (uint8_t i = 0; i < 4; i++) {
            escribir_estado_pin(GPIOB, filas[i], 1);
        }
        // Excitamos solo la fila activa tirándola a tierra (0)
        escribir_estado_pin(GPIOB, filas[f], 0);

        // Retardo ínfimo en ticks que evita variaciones de voltaje ruidosas antes de leer
        for (volatile int retardo = 0; retardo < 300; retardo++);

        // Una vez la fila está lista, monitoreamos las columnas
        for (uint8_t c = 0; c < 4; c++) {
            // Si leemos un 0, significa que el botón conectó el Pull-Up a la tierra de la fila
            if (leer_estado_pin(GPIOB, columnas[c]) == 0) {
                pausa_milisegundos(30); // Lógica de antirrebote para estabilizar la placa metálica del botón
                
                int limite_espera = 150000; // Time-out de seguridad
                // Detenemos la CPU hasta que el usuario levante el dedo del botón o pase el time-out
                while ((leer_estado_pin(GPIOB, columnas[c]) == 0) && (limite_espera > 0)) {
                    limite_espera--;
                }
                // Si confirmó presión, devolvemos el carácter correspondiente a su X/Y
                return matriz_teclas[f][c];
            }
        }
    }
    return '\0'; // Carácter nulo si en todo el barrido no hubo presión
}

// Sincronizador de la matriz estática del juego hacia los registros eléctricos del STM32
void refrescar_leds(void) {
    for (uint8_t i = 0; i < 9; i++) {
        // Conversión matemática lineal a coordenadas 2D (0 a 8 -> 3x3)
        uint8_t fila = i / 3;
        uint8_t columna = i % 3;
        
        // Extraemos quién es el dueño de la celda
        uint8_t estado_celda = obtener_casilla(fila, columna);

        // Uso de operadores ternarios (condicional ? verdadero : falso) para optimizar el código fuente
        // Escribe estado ALTO en el puerto del humano si la memoria dice que le pertenece, si no, lo apaga
        escribir_estado_pin(mapa_leds[i].puerto_j1, mapa_leds[i].pin_j1, (estado_celda == JUGADOR_UNO) ? 1 : 0);
        
        // Aplica el mismo proceso lógico pero referenciando a la CPU (Bot)
        escribir_estado_pin(mapa_leds[i].puerto_j2, mapa_leds[i].pin_j2, (estado_celda == JUGADOR_DOS) ? 1 : 0);
    }
}

// Bucle principal de ejecución desnudo
int main(void) {
    // 1. Configuración de hardware de reloj e interfaces GPIO
    configurar_pines_gpio();
    // 2. Limpieza de memoria del juego
    inicializar_juego();
    // 3. Forzar el apagado de la interfaz visual mandando el tablero vacío a los LEDs
    refrescar_leds();

    // Loop infinito exigido para sistemas embebidos, evita que el procesador caiga en Fault 
    while (1) { 
        char tecla = escanear_teclado_matricial(); // Consulta pasiva constante

        // Acotamos el mapeo únicamente del 1 al 9 físico del pad
        if (tecla >= '1' && tecla <= '9') {
            // Conversión matemática del ASCII char al índice entero plano (0-8)
            uint8_t indice = tecla - '1'; 
            uint8_t fila = indice / 3;
            uint8_t columna = indice % 3;

            // Procesamos la jugada del humano. Si el retorno es true, la jugada procedió.
            if (registrar_movimiento(fila, columna, JUGADOR_UNO)) {
                refrescar_leds(); // Iluminamos inmediatamente su jugada en hardware
                
                // Inspeccionamos la memoria tras la jugada humana. Si no hay ganadores, sigue el bot.
                if (evaluar_estado_juego() == ESTADO_ACTIVO) {
                    pausa_milisegundos(400); // Retardo artificial para dar feedback visual humano de 'turno'
                    turno_maquina();         // Modificación de memoria por la IA
                    refrescar_leds();        // Actualización de diodos emisores
                }
            }
        }

        // Condicional trampa: Se dispara únicamente cuando el estado muta de ESTADO_ACTIVO a victoria/empate
        if (evaluar_estado_juego() != ESTADO_ACTIVO) {
            pausa_milisegundos(2500); // Congela la placa entera 2.5 segundos para exhibir los LEDs ganadores
            inicializar_juego();      // Purga de matriz de software
            refrescar_leds();         // Purga física de los diodos (Reinicio final del hardware)
        }
    }
}