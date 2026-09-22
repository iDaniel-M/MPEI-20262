#include "game.h"

// Tablero interno de 3x3
static uint8_t board[3][3];
static uint8_t current_state;

void game_init(void) {
    for (uint8_t i = 0; i < 3; i++) {
        for (uint8_t j = 0; j < 3; j++) {
            board[i][j] = EMPTY;
        }
    }
    current_state = STATE_PLAYING;
}

uint8_t get_board_cell(uint8_t row, uint8_t col) {
    if (row < 3 && col < 3) {
        return board[row][col];
    }
    return EMPTY;
}

bool make_move(uint8_t row, uint8_t col, uint8_t player) {
    // Verifica si la celda está vacía y el juego sigue activo
    if (row < 3 && col < 3 && board[row][col] == EMPTY && current_state == STATE_PLAYING) {
        board[row][col] = player;
        current_state = check_game_state();
        return true;
    }
    return false;
}

uint8_t check_game_state(void) {
    // 1. Revisar filas y columnas
    for (uint8_t i = 0; i < 3; i++) {
        if (board[i][0] != EMPTY && board[i][0] == board[i][1] && board[i][1] == board[i][2]) {
            return (board[i][0] == PLAYER_HUMAN) ? STATE_HUMAN_WINS : STATE_MCU_WINS;
        }
        if (board[0][i] != EMPTY && board[0][i] == board[1][i] && board[1][i] == board[2][i]) {
            return (board[0][i] == PLAYER_HUMAN) ? STATE_HUMAN_WINS : STATE_MCU_WINS;
        }
    }
    
    // 2. Revisar diagonales
    if (board[0][0] != EMPTY && board[0][0] == board[1][1] && board[1][1] == board[2][2]) {
        return (board[0][0] == PLAYER_HUMAN) ? STATE_HUMAN_WINS : STATE_MCU_WINS;
    }
    if (board[0][2] != EMPTY && board[0][2] == board[1][1] && board[1][1] == board[2][0]) {
        return (board[0][2] == PLAYER_HUMAN) ? STATE_HUMAN_WINS : STATE_MCU_WINS;
    }

    // 3. Revisar si hay empate (tablero lleno)
    bool full = true;
    for (uint8_t i = 0; i < 3; i++) {
        for (uint8_t j = 0; j < 3; j++) {
            if (board[i][j] == EMPTY) {
                full = false;
            }
        }
    }

    if (full) {
        return STATE_DRAW;
    }

    return STATE_PLAYING;
}

void mcu_make_move(void) {
    if (current_state != STATE_PLAYING) {
        return;
    }

    // Lógica básica para el MCU: busca la primera casilla vacía disponible y marca su turno
    for (uint8_t i = 0; i < 3; i++) {
        for (uint8_t j = 0; j < 3; j++) {
            if (board[i][j] == EMPTY) {
                board[i][j] = PLAYER_MCU;
                current_state = check_game_state();
                return;
            }
        }
    }
}