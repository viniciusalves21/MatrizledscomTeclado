#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "pico/bootrom.h"
#include "hardware/pwm.h"

// Arquivo com o programa PIO que controla os LEDs WS2812:
#include "pio_matrix.pio.h"

static const uint ROW_PINS[4] = {8, 9, 6, 5};   // Ajuste conforme seu hardware
static const uint COL_PINS[4] = {4, 3, 2, 28};  // Ajuste conforme seu hardware

static const char KEYPAD[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};


#define NUM_PIXELS 25    // 5x5
#define OUT_PIN    7     // Pino de saída de dados para os LEDs WS2812
#define BUZZER_PIN 21    // Porta associada ao Buzzer


uint32_t matrix_rgb(double r, double g, double b) {
    // Converte cada componente para 0..255
    unsigned char R = (unsigned char)(r * 255);
    unsigned char G = (unsigned char)(g * 255);
    unsigned char B_ = (unsigned char)(b * 255);
    // Monta no formato GRB (24 bits) e descarta o byte menos significativo:
    return ( (uint32_t)(G) << 24 )
         | ( (uint32_t)(R) << 16 )
         | ( (uint32_t)(B_) <<  8 );
}


char read_keypad(void) {
    for (int col = 0; col < 4; col++) {
        // Ativa coluna
        gpio_put(COL_PINS[col], 1);

        for (int row = 0; row < 4; row++) {
            if (gpio_get(ROW_PINS[row])) {
                sleep_ms(10); // debounce simples
                if (gpio_get(ROW_PINS[row])) {
                    // Desativa coluna antes de retornar
                    gpio_put(COL_PINS[col], 0);
                    return KEYPAD[row][col];
                }
            }
        }
        // Desativa coluna antes de continuar
        gpio_put(COL_PINS[col], 0);
    }
    return 0;
}


void init_keypad_pins(void) {
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


void desenha_frame(const double *frame, PIO pio, uint sm, double r, double g, double b) {
    for (int i = 0; i < NUM_PIXELS; i++) {
        double intensidade = frame[i];
        uint32_t cor = matrix_rgb(intensidade * r,
                                  intensidade * g,
                                  intensidade * b);
        pio_sm_put_blocking(pio, sm, cor);
    }
}


void desliga_leds(PIO pio, uint sm) {
    static double frame[NUM_PIXELS] = {0};
    desenha_frame(frame, pio, sm, 0.0, 0.0, 0.0);
}


void liga_leds_cor(PIO pio, uint sm, double r, double g, double b) {
    static double frame[NUM_PIXELS];
    for (int i = 0; i < NUM_PIXELS; i++) {
        frame[i] = 1.0; // acende todos
    }
    desenha_frame(frame, pio, sm, r, g, b);
}
static double animacao0_frames[5][NUM_PIXELS] = {
    {0,0,0,0,0, 0,1,1,1,0, 0,1,1,1,0, 0,1,1,1,0, 0,0,0,0,0},
    {0,0,0,0,0, 1,1,1,0,0, 1,1,1,0,0, 1,1,1,0,0, 0,0,0,0,0},
    {0,0,0,0,0, 1,1,0,0,0, 1,1,0,0,0, 1,1,0,0,0, 0,0,0,0,0},
    {0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0},
    {0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0}
};

static double animacao1_frames[5][NUM_PIXELS] = {
    {1,0,0,0,1, 0,1,0,1,0, 0,0,1,0,0, 0,1,0,1,0, 1,0,0,0,1},
    {0,1,0,1,0, 1,0,1,0,1, 0,1,0,1,0, 1,0,1,0,1, 0,1,0,1,0},
    {0,0,1,0,0, 0,0,1,0,0, 1,1,1,1,1, 0,0,1,0,0, 0,0,1,0,0},
    {0,1,0,1,0, 1,0,1,0,1, 0,1,0,1,0, 1,0,1,0,1, 0,1,0,1,0},
    {1,0,0,0,1, 0,1,0,1,0, 0,0,1,0,0, 0,1,0,1,0, 1,0,0,0,1}
};

static double animacao2_frames[5][NUM_PIXELS] = {
    {1,1,1,1,1, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 1,1,1,1,1},
    {0,0,0,0,0, 1,1,1,1,1, 0,1,0,1,0, 1,1,1,1,1, 0,0,0,0,0},
    {1,0,1,0,1, 0,1,1,1,0, 1,0,1,0,1, 0,1,1,1,0, 1,0,1,0,1},
    {0,0,0,0,0, 0,0,1,0,0, 1,1,1,1,1, 0,0,1,0,0, 0,0,0,0,0},
    {1,1,1,1,1, 1,1,1,1,1, 0,0,1,0,0, 1,1,1,1,1, 1,1,1,1,1}
};

static double animacao3_frames[5][NUM_PIXELS] = {
    {1,0,0,0,0, 0,0,1,0,0, 0,0,0,1,0, 0,0,0,0,1, 1,1,1,1,1},
    {0,1,0,1,0, 1,0,1,0,1, 1,0,1,0,1, 0,1,0,1,0, 1,0,1,0,1},
    {0,0,1,0,0, 1,1,1,1,1, 0,0,1,0,0, 1,1,1,1,1, 0,0,1,0,0},
    {0,1,0,1,0, 1,0,1,0,1, 0,1,0,1,0, 1,0,1,0,1, 0,1,0,1,0},
    {1,0,0,0,1, 0,1,0,1,0, 0,0,1,0,0, 0,1,0,1,0, 1,0,0,0,1}
};
static double animacao4_frames[5][NUM_PIXELS] = {
    {0,0,1,0,0, 0,0,0,0,0, 0,0,1,1,0, 0,1,0,0,0, 0,1,1,1,0},
    {1,1,1,1,1, 1,0,0,0,1, 1,0,1,0,1, 1,0,0,0,1, 1,1,1,1,1},
    {0,0,0,0,0, 0,1,1,1,0, 0,1,0,1,0, 0,1,1,1,0, 0,0,0,0,0},
    {0,0,0,0,0, 0,0,1,0,0, 0,1,1,1,0, 0,0,1,0,0, 0,0,0,0,0},
    {1,0,0,0,1, 0,1,0,1,0, 0,0,1,0,0, 0,1,0,1,0, 1,0,0,0,1}
};
static double animacao5_frames[5][NUM_PIXELS] = {
    {0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 1,1,1,1,1},
    {0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 1,1,1,1,1, 1,1,1,1,1},
    {0,0,0,0,0, 0,0,0,0,0, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1},
    {0,0,0,0,0, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1},
    {1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1}
};
static double animacao6_frames[5][NUM_PIXELS] = {
    {1,0,0,0,0, 1,0,0,0,0, 1,0,0,0,0, 1,0,0,0,0, 1,0,0,0,0},
    {0,1,0,0,0, 0,1,0,0,0, 0,1,0,0,0, 0,1,0,0,0, 0,1,0,0,0},
    {0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0},
    {0,0,0,1,0, 0,0,0,1,0, 0,0,0,1,0, 0,0,0,1,0, 0,0,0,1,0},
    {0,0,0,0,1, 0,0,0,0,1, 0,0,0,0,1, 0,0,0,0,1, 0,0,0,0,1}
};

void reproduz_animacao(double anim[5][NUM_PIXELS], PIO pio, uint sm,
                       double r, double g, double b, int fps)
{
    int delay_ms = 1000 / fps; // conversão de fps para ms
    for (int i = 0; i < 5; i++) {
        desenha_frame(anim[i], pio, sm, r, g, b);
        sleep_ms(delay_ms);
    }
}
//Define as notas
// Frequências das notas em Hz
#define DO 132
#define DO_S 139
#define RE 148
#define RE_S 156
#define MI 166
#define FA 176
#define FA_S 186
#define SOL 197
#define SOL_S 209
#define LA 222
#define LA_S 235
#define SI 249
#define DO_ 264

// Tempo base e intervalo
#define TEMPO 350 // Duração de cada nota
#define INTERVALO 100 // Pausa entre notas

//Inicia o Buzzer
void buzz(uint freq, uint tempo) {
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    uint channel = pwm_gpio_to_channel(BUZZER_PIN);

    // Configurando a frequência
    uint32_t clock_freq = 125000000;
    uint32_t divider = clock_freq / freq / 65536 + 1;
    uint32_t top = clock_freq / (divider * freq);

    // Configurando as repetições
    pwm_set_clkdiv(slice_num, divider);
    pwm_set_wrap(slice_num, top - 1);
    pwm_set_chan_level(slice_num, channel, top / 2);
    pwm_set_enabled(slice_num, true);

    sleep_ms(tempo);

    pwm_set_enabled(slice_num, false);
}
//Toca as notas
int Tocar() {
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);

    // Sequência de notas
    buzz(FA_S, TEMPO); sleep_ms(INTERVALO); // Fá#
    buzz(RE, TEMPO); sleep_ms(INTERVALO); // Ré
    buzz(RE, TEMPO); sleep_ms(INTERVALO); // Ré
    buzz(MI, TEMPO); sleep_ms(INTERVALO); // Mi
    buzz(FA, TEMPO); sleep_ms(INTERVALO); // Fá
    buzz(MI, TEMPO); sleep_ms(INTERVALO); // Mi
    buzz(RE, TEMPO); sleep_ms(INTERVALO); // Ré
    buzz(DO_S, TEMPO); sleep_ms(INTERVALO); // Dó#
    buzz(RE, TEMPO); sleep_ms(INTERVALO); // Ré
    buzz(MI, TEMPO); sleep_ms(INTERVALO); // Mi
    buzz(FA_S, TEMPO); sleep_ms(INTERVALO); // Fá#
    buzz(SI, TEMPO); sleep_ms(INTERVALO); // Si
    buzz(SI, TEMPO); sleep_ms(INTERVALO); // Si
    buzz(DO_S, TEMPO); sleep_ms(INTERVALO); // Dó#
    buzz(RE, TEMPO); sleep_ms(INTERVALO); // Ré
    buzz(MI, TEMPO); sleep_ms(INTERVALO); // Mi
    buzz(RE, TEMPO); sleep_ms(INTERVALO); // Ré
    buzz(DO_S, TEMPO); sleep_ms(INTERVALO); // Dó#
    buzz(LA, TEMPO); sleep_ms(INTERVALO); // Lá
    buzz(SOL, TEMPO); sleep_ms(INTERVALO); // Sol
    buzz(FA_S, TEMPO); sleep_ms(INTERVALO); // Fá#

    return 0;
}
int main() {
    stdio_init_all();
    init_keypad_pins();

    // Inicializa a PIO e o state machine
    PIO pio = pio0;
    uint sm = pio_claim_unused_sm(pio, true);
    uint offset = pio_add_program(pio, &pio_matrix_program);
    pio_matrix_program_init(pio, sm, offset, OUT_PIN);

    printf("Sistema iniciado. Aguardando teclas...\n");

    while (true) {
        char key = read_keypad();
        if (key != 0) {
            printf("Tecla pressionada: %c\n", key);

            switch (key) {
                case '0':
                    reproduz_animacao(animacao0_frames, pio, sm,
                                      0.0, 0.0, 1.0, 10); // Azul, 10 fps
                    break;

                case '1':
                    reproduz_animacao(animacao1_frames, pio, sm,
                                      1.0, 0.0, 0.0, 10); // Vermelho, 10 fps
                    break;
                case '2':
                    reproduz_animacao(animacao2_frames, pio, sm,
                                      0.0, 1.0, 0.0, 8);  // cor verde, 8 fps
                    break;
                case '3':
                    reproduz_animacao(animacao3_frames, pio, sm,
                                      0.3, 0.5, 1.0, 8); // Azul, 8 Fps
                    break;
                case '4':
                    reproduz_animacao(animacao4_frames, pio, sm,
                                      0.5, 0.5, 0.0, 8); // Amarelo,  8 Fps
                    break;
                case '5':
                    reproduz_animacao(animacao5_frames, pio, sm,
                                      0.9, 0.6, 0.0, 9); // Laranja, 9 Fps
                    break;
                case '6':
                    reproduz_animacao(animacao6_frames, pio, sm,
                                      0.5, 0.0, 0.5, 10); // Rosa, 10 Fps

                    Tocar();

                    reproduz_animacao(animacao6_frames, pio, sm,
                                      0.5, 0.0, 0.5, 10); // Rosa, 10 Fps

                                      
                    break;
                case '7':
                case '8':
                case '9':
                    desliga_leds(pio, sm);
                    printf("Animação %c não definida: Desligando LEDs.\n", key);
                    break;

                // Tecla A => Desliga
                case 'A':
                    desliga_leds(pio, sm);
                    break;
                // Tecla B => Liga tudo em azul (100%)
                case 'B':
                    liga_leds_cor(pio, sm, 0.0, 0.0, 1.0);
                    break;
                // Tecla C => Liga tudo em vermelho (80%)
                case 'C':
                    liga_leds_cor(pio, sm, 0.8, 0.0, 0.0);
                    break;
                // Tecla D => Liga tudo em verde (50%)
                case 'D':
                    liga_leds_cor(pio, sm, 0.0, 0.5, 0.0);
                    break;
                // Tecla # => Liga tudo em branco (20%)
                case '#':
                    liga_leds_cor(pio, sm, 0.2, 0.2, 0.2);
                    break;
                default:
                    break;
            }

            while (read_keypad() != 0) {
                tight_loop_contents();
            }
        }
        sleep_ms(50);
    }
    return 0;
}