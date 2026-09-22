#include "game.h"

// Matriz estatica que representa la memoria del tablero de 3x3
static uint8_t tablero[3][3];
// Variable que almacena el estado actual de la partida
static uint8_t estado_actual;

// Función para limpiar la matriz y empezar el juego de cero
void inicializar_juego(void) {
    // Recorremos las filas con la variable 'f'
    for (uint8_t f = 0; f < 3; f++) {
        // Recorremos las columnas con la variable 'c'
        for (uint8_t c = 0; c < 3; c++) {
            tablero[f][c] = CASILLA_VACIA; // Asignamos un cero a cada posicion
        }
    }
    estado_actual = ESTADO_ACTIVO; // Declaramos la partida como inicializada y en curso
}

// Función para consultar que jugador tiene una celda en específico
uint8_t obtener_casilla(uint8_t fila, uint8_t col) {
    // Verificamos por seguridad que las coordenadas no superen el indice maximo (2)
    if (fila < 3 && col < 3) {
        return tablero[fila][col];
    }
    return CASILLA_VACIA; // Retorno de seguridad ante índices no validos
}

// Funcion encargada de escribir la ficha de un jugador en la matriz
bool registrar_movimiento(uint8_t fila, uint8_t col, uint8_t jugador) {
    // Validamos limites, que la casilla este libre y que la partida no haya acabado
    if (fila < 3 && col < 3 && tablero[fila][col] == CASILLA_VACIA && estado_actual == ESTADO_ACTIVO) {
        tablero[fila][col] = jugador; // Escribimos la ficha en memoria
        estado_actual = evaluar_estado_juego(); // Comprobamos si esta jugada desencadeno una victoria o empate
        return true; // Indicamos que el movimiento fue procesado exitosamente
    }
    return false; // El movimiento no fue valido
}

// Logica para revisar victoria, empate o continuación
uint8_t evaluar_estado_juego(void) {
    // 1. Verificamos coincidencia en filas o columnas completas
    for (uint8_t x = 0; x < 3; x++) {
        // Chequeo de filas (revisión horizontal)
        if (tablero[x][0] != CASILLA_VACIA && tablero[x][0] == tablero[x][1] && tablero[x][1] == tablero[x][2]) {
            return (tablero[x][0] == JUGADOR_UNO) ? ESTADO_GANA_J1 : ESTADO_GANA_J2;
        }
        // Chequeo de columnas (revisión vertical)
        if (tablero[0][x] != CASILLA_VACIA && tablero[0][x] == tablero[1][x] && tablero[1][x] == tablero[2][x]) {
            return (tablero[0][x] == JUGADOR_UNO) ? ESTADO_GANA_J1 : ESTADO_GANA_J2;
        }
    }
    
    // 2. Verificamos coincidencia en las dos diagonales principales
    // Diagonal principal (\)
    if (tablero[0][0] != CASILLA_VACIA && tablero[0][0] == tablero[1][1] && tablero[1][1] == tablero[2][2]) {
        return (tablero[0][0] == JUGADOR_UNO) ? ESTADO_GANA_J1 : ESTADO_GANA_J2;
    }
    // Diagonal secundaria (/)
    if (tablero[0][2] != CASILLA_VACIA && tablero[0][2] == tablero[1][1] && tablero[1][1] == tablero[2][0]) {
        return (tablero[0][2] == JUGADOR_UNO) ? ESTADO_GANA_J1 : ESTADO_GANA_J2;
    }

    // 3. Verificamos si hay empate contando las casillas vacías (lógica cambiada frente al original)
    uint8_t casillas_vacias = 0;
    for (uint8_t f = 0; f < 3; f++) {
        for (uint8_t c = 0; c < 3; c++) {
            if (tablero[f][c] == CASILLA_VACIA) {
                casillas_vacias++; // Contabilizamos los huecos disponibles
            }
        }
    }

    // Si ya no quedan huecos y ninguna validación anterior dio un ganador, es empate
    if (casillas_vacias == 0) {
        return ESTADO_EMPATE;
    }

    // Si la matriz no está llena y nadie ha ganado, se sigue jugando
    return ESTADO_ACTIVO;
}

// Inteligencia artificial basica para la máquina
void turno_maquina(void) {
    // Verificación de seguridad para no alterar la memoria de una partida terminada
    if (estado_actual != ESTADO_ACTIVO) {
        return;
    }

    // buscamos casillas de reversa (desde abajo-derecha hasta arriba-izquierda)
    for (int8_t f = 2; f >= 0; f--) {
        for (int8_t c = 2; c >= 0; c--) {
            // Evaluamos si encontramos la primera celda libre desde el final
            if (tablero[f][c] == CASILLA_VACIA) {
                tablero[f][c] = JUGADOR_DOS; // Ejecutamos la jugada de la máquina
                estado_actual = evaluar_estado_juego(); // Revalidamos la condición de victoria
                return; // Matamos la ejecucion para que juegue solo una vez por turno
            }
        }
    }
}