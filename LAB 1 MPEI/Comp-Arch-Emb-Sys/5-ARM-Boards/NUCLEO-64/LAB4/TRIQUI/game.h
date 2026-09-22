#ifndef GAME_H
#define GAME_H

#include <stdint.h>  // Libreria para utilizar tipos de datos de tamaño fijo (ej. uint8_t)
#include <stdbool.h> // Libreria para utilizar booleanos (true/false)

// Definimos las constantes para las casillas usando nombres distintos a los originales
#define CASILLA_VACIA 0
#define JUGADOR_UNO   1 
#define JUGADOR_DOS   2 

// Definimos los estados del juego con nueva nomenclatura
#define ESTADO_ACTIVO  0
#define ESTADO_GANA_J1 1
#define ESTADO_GANA_J2 2
#define ESTADO_EMPATE  3

// Prototipos de las funciones que manejan la logica interna
void inicializar_juego(void);
uint8_t obtener_casilla(uint8_t fila, uint8_t col);
bool registrar_movimiento(uint8_t fila, uint8_t col, uint8_t jugador);
uint8_t evaluar_estado_juego(void);
void turno_maquina(void);

#endif