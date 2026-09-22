#include <stdint.h>
#include "stm32f401.h"
#include "game.h"
#include "gpio_config.h"

// Se rediseña la estructura para que los nombres sean mas claros
typedef struct {
    GPIO_TypeDef *puerto_j1; // Registro base del puerto para el Jugador 1
    uint8_t pin_j1;          // Numero de pin exacto del Jugador 1
    
    GPIO_TypeDef *puerto_j2; // Registro base del puerto para el Bot
    uint8_t pin_j2;          // Numero de pin exacto del Bot
} MapeoCasillas;

// Array estructural constante que vincula la logica estricta de hardware de los 18 pines a las 9 casillas logicas
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

// Implementacion de retardo crudo que gasta ciclos de reloj quemandolos en un bucle while
void pausa_milisegundos(volatile uint32_t ms) {
    // El procesador a 16MHz estandar gasta aproximandamente 4 ciclos por instruccion. Esto empareja la temporalidad.
    uint32_t ciclos = ms * 4000; 
    while (ciclos > 0) {
        __asm("nop"); // Ensamblador puro (No-Operation) que instruye a la CPU a hacer nada un ciclo
        ciclos--;
    }
}

// Funcion encargada del multiplexado de las teclas
char escanear_teclado_matricial(void) {
    // Distribucion fisica del teclado membrana
    const char matriz_teclas[4][4] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
    };

    uint8_t filas[4] = {1, 0, 12, 15};
    uint8_t columnas[4] = {5, 6, 7, 8};

    // Escaneo de filas por multiplexacion
    for (uint8_t f = 0; f < 4; f++) {
        // Establecemos en alto (1) todas las filas para anular lecturas falsas
        for (uint8_t i = 0; i < 4; i++) {
            escribir_estado_pin(GPIOB, filas[i], 1);
        }
        // Activamos solo la fila activa tirandola a tierra (0)
        escribir_estado_pin(GPIOB, filas[f], 0);

        // Retardo infimo en ticks que evita variaciones de voltaje ruidosas antes de leer
        for (volatile int retardo = 0; retardo < 300; retardo++);

        // Una vez la fila esta lista, monitoreamos las columnas
        for (uint8_t c = 0; c < 4; c++) {
            // Si leemos un 0, significa que el boton conecto el Pull-Up a la tierra de la fila
            if (leer_estado_pin(GPIOB, columnas[c]) == 0) {
                pausa_milisegundos(30); // Logica de antirrebote para estabilizar la placa del boton
                
                int limite_espera = 150000; // Time-out de seguridad
                // Detenemos la CPU hasta que el usuario levante el dedo del boton o pase el time-out
                while ((leer_estado_pin(GPIOB, columnas[c]) == 0) && (limite_espera > 0)) {
                    limite_espera--;
                }
                // Si confirmo presion, devolvemos el caracter correspondiente a su X/Y
                return matriz_teclas[f][c];
            }
        }
    }
    return '\0'; // Caracter nulo si en todo el barrido no hubo presion
}

// Sincronizador de la matriz estatica del juego hacia los registros electricos del STM32
void refrescar_leds(void) {
    for (uint8_t i = 0; i < 9; i++) {
        // Conversion matematica lineal a coordenadas 2D (0 a 8 -> 3x3)
        uint8_t fila = i / 3;
        uint8_t columna = i % 3;
        
        // Extraemos quien es el dueño de la celda
        uint8_t estado_celda = obtener_casilla(fila, columna);
        
        // Uso de operadores ternarios (condicional ? verdadero : falso)
        // Escribe estado ALTO en el puerto del humano si la memoria dice que le pertenece, si no, lo apaga
        escribir_estado_pin(mapa_leds[i].puerto_j1, mapa_leds[i].pin_j1, (estado_celda == JUGADOR_UNO) ? 1 : 0);
        
        // Aplica el mismo proceso logico pero referenciando a la CPU (Bot)
        escribir_estado_pin(mapa_leds[i].puerto_j2, mapa_leds[i].pin_j2, (estado_celda == JUGADOR_DOS) ? 1 : 0);
    }
}

// Bucle principal de ejecucion desnudo
int main(void) {
    // 1. Configuracion de hardware de reloj e interfaces GPIO
    configurar_pines_gpio();
    // 2. Limpieza de memoria del juego
    inicializar_juego();
    // 3. Forzar el apagado de la interfaz visual mandando el tablero vacio a los LEDs
    refrescar_leds();

    // Loop infinito exigido para sistemas embebidos, evita que el procesador caiga en Fault 
    while (1) { 
        char tecla = escanear_teclado_matricial(); // Consulta pasiva constante

        // Acotamos el mapeo unicamente del 1 al 9 fisico del pad
        if (tecla >= '1' && tecla <= '9') {
            // Conversion matematica del ASCII char al indice entero plano (0-8)
            uint8_t indice = tecla - '1'; 
            uint8_t fila = indice / 3;
            uint8_t columna = indice % 3;

            // Procesamos la jugada del humano. Si el retorno es true, la jugada procedio.
            if (registrar_movimiento(fila, columna, JUGADOR_UNO)) {
                refrescar_leds(); // Iluminamos inmediatamente su jugada en hardware
                
                // Inspeccionamos la memoria tras la jugada humana. Si no hay ganadores, sigue el bot.
                if (evaluar_estado_juego() == ESTADO_ACTIVO) {
                    pausa_milisegundos(250); // Retardo artificial para dar feedback visual humano de 'turno'
                    turno_maquina();         // Modificacion de memoria por la IA
                    refrescar_leds();        // Actualizacion de diodos emisores
                }
            }
        }

        // Condicional trampa: Se dispara unicamente cuando el estado muta de ESTADO_ACTIVO a victoria/empate
        if (evaluar_estado_juego() != ESTADO_ACTIVO) {
            pausa_milisegundos(2500); // Congela la placa entera 2.5 segundos para exhibir los LEDs ganadores
            inicializar_juego();      // Purga de matriz de software
            refrescar_leds();         // Purga fisica de los diodos (Reinicio final del hardware)
        }
    }
}