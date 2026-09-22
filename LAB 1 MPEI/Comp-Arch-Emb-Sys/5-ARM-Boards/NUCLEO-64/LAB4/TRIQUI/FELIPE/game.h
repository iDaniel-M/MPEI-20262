#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include <stdbool.h>

// Definición de jugadores y estados
#define EMPTY         0
#define PLAYER_HUMAN  1
#define PLAYER_MCU    2

#define STATE_PLAYING     0
#define STATE_HUMAN_WINS  1
#define STATE_MCU_WINS    2
#define STATE_DRAW        3

// Prototipos de funciones de la lógica del juego
void game_init(void);
uint8_t get_board_cell(uint8_t row, uint8_t col);
bool make_move(uint8_t row, uint8_t col, uint8_t player);
uint8_t check_game_state(void);
void mcu_make_move(void);

#endif