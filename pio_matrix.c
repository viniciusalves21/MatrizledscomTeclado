#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "pico/bootrom.h"
#include "pio_matrix.pio.h"

// --- Definições do Teclado ---
static const uint ROW_PINS[4] = {8, 9, 6, 5};
static const uint COL_PINS[4] = {4, 3, 2, 28};
static const char KEYPAD[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

// --- Definições da Matriz de LEDs ---
#define NUM_PIXELS 25
#define OUT_PIN 7

// --- Definições para as animações ---
double animacao1_frames[5][NUM_PIXELS] = {
    {0,0,0,0,0, 0,1,1,1,0, 0,1,1,1,0, 0,1,1,1,0, 0,0,0,0,0},
    {0,0,0,0,0, 1,1,1,0,0, 1,1,1,0,0, 1,1,1,0,0, 0,0,0,0,0},
    {0,0,0,0,0, 1,1,0,0,0, 1,1,0,0,0, 1,1,0,0,0, 0,0,0,0,0},
    {0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0},
    {0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0}
};

double animacao2_frames[5][NUM_PIXELS] = {
    {1,0,0,0,1, 0,1,0,1,0, 0,0,1,0,0, 0,1,0,1,0, 1,0,0,0,1},
    {0,1,0,1,0, 1,0,1,0,1, 0,1,0,1,0, 1,0,1,0,1, 0,1,0,1,0},
    {0,0,1,0,0, 0,0,1,0,0, 1,1,1,1,1, 0,0,1,0,0, 0,0,1,0,0},
    {0,1,0,1,0, 1,0,1,0,1, 0,1,0,1,0, 1,0,1,0,1, 0,1,0,1,0},
    {1,0,0,0,1, 0,1,0,1,0, 0,0,1,0,0, 0,1,0,1,0, 1,0,0,0,1}
};

double animacao3_frames[5][NUM_PIXELS] = {
    {0,0,0,0,0, 0,1,1,1,0, 0,1,1,1,0, 0,1,1,1,0, 0,0,0,0,0},
    {0,0,0,0,0, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 0,0,0,0,0},
    {0,0,0,0,0, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 0,0,0,0,0},
    {0,0,0,0,0, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 0,0,0,0,0},
    {0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0}
};

// --- Funções da Matriz ---
uint32_t matrix_rgb(double b, double r, double g) {
    return (uint32_t)((uint8_t)(g * 255) << 16 | (uint8_t)(r * 255) << 8 | (uint8_t)(b * 255));
}

void desenha_frame(double *frame, PIO pio, uint sm, double r, double g, double b) {
    for (int16_t i = 0; i < NUM_PIXELS; i++) {
        uint32_t valor_led = matrix_rgb(frame[i]*b, frame[i]*r, frame[i]*g);
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}

// --- Funções do Teclado ---
char read_keypad(void) {
    for (int col = 0; col < 4; col++) {
        gpio_put(COL_PINS[col], 1);
        for (int row = 0; row < 4; row++) {
            if (gpio_get(ROW_PINS[row])) {
                sleep_ms(10);
                if (gpio_get(ROW_PINS[row])) {
                    gpio_put(COL_PINS[col], 0);
                    return KEYPAD[row][col];
                }
            }
        }
        gpio_put(COL_PINS[col], 0);
    }
    return 0;
}

void init_pins(void) {
    stdio_init_all();

    for (int i = 0; i < 4; i++) {
        gpio_init(ROW_PINS[i]);
        gpio_set_dir(ROW_PINS[i], GPIO_IN);
        gpio_pull_down(ROW_PINS[i]);
    }

    for (int i = 0; i < 4; i++) {
        gpio_init(COL_PINS[i]);
        gpio_set_dir(COL_PINS[i], GPIO_OUT);
        gpio_put(COL_PINS[i], 0);
    }
}

// --- Funções de Controle dos LEDs ---
void desliga_leds(PIO pio, uint sm) {
    double frame[NUM_PIXELS] = {0};
    desenha_frame(frame, pio, sm, 0, 0, 0);
}

void liga_leds_cor(PIO pio, uint sm, double r, double g, double b) {
    double frame[NUM_PIXELS];
    for(int i = 0; i < NUM_PIXELS; i++){
        frame[i] = 1;
    }
    desenha_frame(frame, pio, sm, r, g, b);
}

int main() {
    PIO pio = pio0;
    uint sm = pio_claim_unused_sm(pio, true);
    uint offset = pio_add_program(pio, &pio_matrix_program);
    pio_matrix_program_init(pio, sm, offset, OUT_PIN);

    stdio_init_all();
    init_pins();

    while (true) {
        char key = read_keypad();

        if (key != 0) {
            printf("Tecla pressionada: %c\n", key);

            switch (key) {
                case '1': // Animação 1 (Vermelho)
                    for (int i = 0; i < 5; i++) {
                        desenha_frame(animacao1_frames[i], pio, sm, 1.0, 0.0, 0.0);
                        sleep_ms(100);
                    }
                    break;
                case '2': // Animação 2 (Verde)
                    for (int i = 0; i < 5; i++) {
                        desenha_frame(animacao2_frames[i], pio, sm, 0.0, 1.0, 0.0);
                        sleep_ms(100);
                    }
                    break;
                case '3': // Animação 3 (Azul)
                    for (int i = 0; i < 5; i++) {
                        desenha_frame(animacao3_frames[i], pio, sm, 0.0, 0.0, 1.0);
                        sleep_ms(100);
                    }
                    break;
                case 'A': // Desliga todos os LEDs
                    desliga_leds(pio, sm);
                    break;
                case 'B': // Liga todos os LEDs em Azul (100%)
                    liga_leds_cor(pio, sm, 0.0, 0.0, 1.0);
                    break;
                case 'C': // Liga todos os LEDs em Vermelho (80%)
                    liga_leds_cor(pio, sm, 0.8, 0.0, 0.0);
                    break;
                case 'D': // Liga todos os LEDs em Verde (50%)
                    liga_leds_cor(pio, sm, 0.0, 0.5, 0.0);
                    break;
                case '#': // Liga todos os LEDs em Branco (20%)
                    liga_leds_cor(pio, sm, 0.2, 0.2, 0.2);
                    break;
                default:
                    break;
            }

            // Espera a tecla ser solta para evitar repetições
            while (read_keypad() != 0) {
                tight_loop_contents();
            }
        }
        sleep_ms(50); // Pequena pausa
    }
    return 0;
}