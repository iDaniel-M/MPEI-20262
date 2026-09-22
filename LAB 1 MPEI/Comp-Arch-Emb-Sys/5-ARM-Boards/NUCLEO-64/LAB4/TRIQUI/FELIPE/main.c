#include <stdint.h>
#include "stm32f401.h"
#include "game.h"
#include "gpio_config.h"

// Estructura para asociar cada celda del 1 al 9 con sus pines de salida exactos
typedef struct {
    GPIO_TypeDef *human_port;
    uint8_t human_pin;
    
    GPIO_TypeDef *mcu_port;
    uint8_t mcu_pin;
} CellOutputMap;

// Mapeo exacto validado:
// Celda 1 -> Humano: A2  | Máquina: A6
// Celda 2 -> Humano: A15 | Máquina: A10
// Celda 3 -> Humano: C13 | Máquina: A1
// Celda 4 -> Humano: A3  | Máquina: A5
// Celda 5 -> Humano: A11 | Máquina: A8
// Celda 6 -> Humano: C15 | Máquina: A0
// Celda 7 -> Humano: A4  | Máquina: A7
// Celda 8 -> Humano: A12 | Máquina: A9
// Celda 9 -> Humano: C14 | Máquina: B9
const CellOutputMap cell_outputs[9] = {
    { GPIOA, 2,  GPIOA, 6  }, // Celda 1
    { GPIOA, 15, GPIOA, 10 }, // Celda 2
    { GPIOC, 13, GPIOA, 1  }, // Celda 3
    { GPIOA, 3,  GPIOA, 5  }, // Celda 4
    { GPIOA, 11, GPIOA, 8  }, // Celda 5
    { GPIOC, 15, GPIOA, 0  }, // Celda 6
    { GPIOA, 4,  GPIOA, 7  }, // Celda 7
    { GPIOA, 12, GPIOA, 9  }, // Celda 8
    { GPIOC, 14, GPIOB, 9  }  // Celda 9
};

void delay_ms(volatile uint32_t ms) {
    for (uint32_t i = 0; i < ms * 4000; i++) {
        __asm("nop");
    }
}

// Función para escanear el teclado matricial 4x4 (GPIOB)
char scan_keypad(void) {
    const char keys[4][4] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
    };

    uint8_t row_pins[] = {1, 0, 12, 15};
    uint8_t col_pins[] = {5, 6, 7, 8};

    for (int r = 0; r < 4; r++) {
        for (int i = 0; i < 4; i++) {
            write_pin_state(GPIOB, row_pins[i], 1);
        }
        write_pin_state(GPIOB, row_pins[r], 0);

        for (volatile int d = 0; d < 300; d++);

        for (int c = 0; c < 4; c++) {
            if (read_pin_state(GPIOB, col_pins[c]) == 0) {
                delay_ms(30); // Antirrebote
                int timeout = 150000;
                while ((read_pin_state(GPIOB, col_pins[c]) == 0) && (timeout > 0)) {
                    timeout--;
                }
                return keys[r][c];
            }
        }
    }
    return '\0';
}

// Actualizar el estado físico de los 18 LEDs según la matriz lógica del juego
void update_led_display(void) {
    for (int i = 0; i < 9; i++) {
        uint8_t r = i / 3;
        uint8_t c = i % 3;
        uint8_t cell_status = get_board_cell(r, c);

        // Control LED del Jugador Humano
        if (cell_status == PLAYER_HUMAN) {
            write_pin_state(cell_outputs[i].human_port, cell_outputs[i].human_pin, 1);
        } else {
            write_pin_state(cell_outputs[i].human_port, cell_outputs[i].human_pin, 0);
        }

        // Control LED de la Máquina (MCU)
        if (cell_status == PLAYER_MCU) {
            write_pin_state(cell_outputs[i].mcu_port, cell_outputs[i].mcu_pin, 1);
        } else {
            write_pin_state(cell_outputs[i].mcu_port, cell_outputs[i].mcu_pin, 0);
        }
    }
}

int main(void) {
    // Inicializar pines mediante el HAL y la lógica del Triqui
    GPIO_Config();
    game_init();
    update_led_display();

    while (1) {
        char key = scan_keypad();

        // Las teclas '1' a '9' corresponden directamente a las casillas del tablero
        if (key >= '1' && key <= '9') {
            uint8_t cell_num = key - '1'; 
            uint8_t row = cell_num / 3;
            uint8_t col = cell_num % 3;

            // Turno del jugador humano
            if (make_move(row, col, PLAYER_HUMAN)) {
                update_led_display();
                
                // Si la partida sigue activa, la máquina responde automáticamente
                if (check_game_state() == STATE_PLAYING) {
                    delay_ms(400); 
                    mcu_make_move();
                    update_led_display();
                }
            }
        }

        // Si hay victoria o empate, pausar unos segundos y reiniciar el juego
        if (check_game_state() != STATE_PLAYING) {
            delay_ms(2500); 
            game_init();
            update_led_display();
        }
    }
}