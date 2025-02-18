#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdio.h>
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "ws2812.pio.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

#define LED_MATRIX_WIDTH 5
#define LED_MATRIX_HEIGHT 5
#define LED_PIN 7
#define JOYSTICK_THRESHOLD 2000 // Limite para reduzir a sensibilidade
#define MOVEMENT_SPEED 1        // Velocidade de movimento do LED
#define BUTTON_A_PIN 5          // Botão A (GPIO 5)
#define BUTTON_B_PIN 6          // Botão B (GPIO 6)
#define GREEN_LED_PIN 11        // LED verde (GPIO 11)
#define RED_LED_PIN 13          // LED vermelho (GPIO 13)
#define BUZZER_PIN 21            // Pino do Buzzer
#define SEQUENCE_MAX_LENGTH 10  // Comprimento máximo da sequência
#define DELAY_BETWEEN_STEPS 500 // Delay entre os passos da sequência

// Configurações do display OLED SSD1306
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define OLED_ADDRESS 0x3C

// Cores predefinidas para cada fase
#define NUM_COLORS 6
uint32_t colors[NUM_COLORS] = {
    0x0000FF, // Azul (Fase 1)
    0xFFFF00, // Amarelo (Fase 2)
    0xFF00FF, // Magenta (Fase 3)
    0x00FFFF, // Ciano (Fase 4)
    0x00FF00, // Verde (Fase 5)
    0xFF0000  // Vermelho (Fase 6)
};

// Fator de escala para reduzir a intensidade
float intensity_scale = 0.1;

// Função para tocar um som com frequência e duração específicas
void tocarSom(int frequencia, int duracao_ms) {
    if (frequencia == 0) {
        gpio_put(BUZZER_PIN, 0); // Desliga o buzzer se a frequência for 0
        return;
    }

    // Configura o PWM para gerar a frequência desejada
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN); // Obtém o slice do PWM para o pino do buzzer
    uint channel_num = pwm_gpio_to_channel(BUZZER_PIN); // Obtém o canal do PWM para o pino do buzzer

    // Configura a frequência do PWM
    pwm_set_clkdiv(slice_num, 125.0f); // Define o divisor de clock (ajuste conforme necessário)
    pwm_set_wrap(slice_num, 1000000 / frequencia); // Define o período do PWM com base na frequência
    pwm_set_chan_level(slice_num, channel_num, 10000 / frequencia); // Duty cycle

    pwm_set_enabled(slice_num, true); // Habilita o PWM
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM); // Configura o pino do buzzer para função PWM

    sleep_ms(duracao_ms); // Mantém o som pelo tempo especificado

    pwm_set_enabled(slice_num, false); // Desabilita o PWM
    gpio_put(BUZZER_PIN, 0); // Garante que o buzzer seja desligado
}

// Protótipo da função display_message
void display_message(ssd1306_t *ssd, const char *line1, const char *line2);

// Declaração da função map_led_index
int map_led_index(int x, int y);

// Função para inicializar o ADC
void my_adc_init() {
    adc_init();
    adc_gpio_init(26); // GPIO 26 (Y)
    adc_gpio_init(27); // GPIO 27 (X)
}

// Função para ler o valor do ADC
uint16_t read_adc(uint gpio) {
    adc_select_input(gpio - 26); // ADC0 para GPIO 26, ADC1 para GPIO 27
    return adc_read();
}

// Função para mapear as coordenadas (x, y) para o índice do LED na matriz
int map_led_index(int x, int y) {
    if (y % 2 == 0) {
        // Linhas pares: ordem crescente (esquerda para direita)
        return y * LED_MATRIX_WIDTH + x;
    } else {
        // Linhas ímpares: ordem decrescente (direita para esquerda)
        return y * LED_MATRIX_WIDTH + (LED_MATRIX_WIDTH - 1 - x);
    }
}

// Função para exibir a fase no display OLED
void display_level(ssd1306_t *ssd, int level) {
    char level_message[20];
    sprintf(level_message, "Fase %d", level); // Cria a mensagem da fase
    display_message(ssd, level_message, "Boa Sorte!");
}

// Função para definir a cor de um LED na matriz com intensidade ajustada
void set_led_color(uint32_t *leds, int x, int y, uint32_t color) {
    int index = map_led_index(x, y);
    if (index >= 0 && index < LED_MATRIX_WIDTH * LED_MATRIX_HEIGHT) {
        // Extrai os componentes R, G, B da cor
        uint8_t r = (color >> 16) & 0xFF;
        uint8_t g = (color >> 8) & 0xFF;
        uint8_t b = color & 0xFF;

        // Aplica o fator de escala para reduzir a intensidade
        r = (uint8_t)(r * intensity_scale);
        g = (uint8_t)(g * intensity_scale);
        b = (uint8_t)(b * intensity_scale);

        // Recombina os componentes em uma cor de 32 bits
        leds[index] = (r << 16) | (g << 8) | b;
    }
}

// Função para limpar a matriz de LEDs
void clear_leds(uint32_t *leds) {
    for (int i = 0; i < LED_MATRIX_WIDTH * LED_MATRIX_HEIGHT; i++) {
        leds[i] = 0;
    }
}

// Função para exibir a sequência de LEDs
void show_sequence(uint32_t *leds, int sequence[][2], int sequence_length, uint32_t color) {
    for (int i = 0; i < sequence_length; i++) {
        int x = sequence[i][0];
        int y = sequence[i][1];
        set_led_color(leds, x, y, color); // Usa a cor correspondente à fase
        for (int j = 0; j < LED_MATRIX_WIDTH * LED_MATRIX_HEIGHT; j++) {
            pio_sm_put_blocking(pio0, 0, leds[j] << 8u);
        }
        sleep_ms(DELAY_BETWEEN_STEPS);
        clear_leds(leds);
        for (int j = 0; j < LED_MATRIX_WIDTH * LED_MATRIX_HEIGHT; j++) {
            pio_sm_put_blocking(pio0, 0, leds[j] << 8u);
        }
        sleep_ms(DELAY_BETWEEN_STEPS / 2);
    }
}

// Função para gerar uma sequência aleatória
void generate_sequence(int sequence[][2], int sequence_length) {
    for (int i = 0; i < sequence_length; i++) {
        sequence[i][0] = rand() % LED_MATRIX_WIDTH;  // Posição X aleatória
        sequence[i][1] = rand() % LED_MATRIX_HEIGHT;   // Posição Y aleatória
    }
}

// Função para exibir uma mensagem no display OLED
void display_message(ssd1306_t *ssd, const char *line1, const char *line2) {
    ssd1306_fill(ssd, false);  // Limpa a tela

    // Exibe a primeira linha (parte superior)
    ssd1306_draw_string(ssd, line1, 10, 10); // (x, y) = (10, 10)

    // Exibe a segunda linha (parte inferior)
    ssd1306_draw_string(ssd, line2, 10, 30); // (x, y) = (10, 30)

    ssd1306_send_data(ssd); // Envia os dados para o display
}

// Função principal
int main() {
    stdio_init_all();

    // Inicializa o ADC
    my_adc_init();

    // Inicializa a matriz de LEDs
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, LED_PIN, 800000, false);

    // Inicializa os botões e os LEDs
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);

    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);

    gpio_init(GREEN_LED_PIN);
    gpio_set_dir(GREEN_LED_PIN, GPIO_OUT);

    gpio_init(RED_LED_PIN);
    gpio_set_dir(RED_LED_PIN, GPIO_OUT);

    // Inicializa o buzzer
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    gpio_put(BUZZER_PIN, 0);

    // Inicializa o display OLED
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_t ssd;
    ssd1306_init(&ssd, 128, 64, false, OLED_ADDRESS, I2C_PORT);
    ssd1306_fill(&ssd, false); // Limpa a tela
    ssd1306_send_data(&ssd);   // Envia os dados para o display

    uint32_t leds[LED_MATRIX_WIDTH * LED_MATRIX_HEIGHT] = {0};

    int sequence[SEQUENCE_MAX_LENGTH][2]; // Armazena a sequência de posições
    int sequence_length = 2;              // Comprimento inicial da sequência
    int level = 1;                        // Nível atual

    // Exibe a fase 1 no início
    display_level(&ssd, level);

    while (true) {
        generate_sequence(sequence, sequence_length);

        // Obtém a cor correspondente à fase atual
        uint32_t current_color = colors[(level - 1) % NUM_COLORS];

        // Exibe a sequência para o jogador
        show_sequence(leds, sequence, sequence_length, current_color);

        // Limpa a matriz de LEDs
        clear_leds(leds);
        for (int i = 0; i < LED_MATRIX_WIDTH * LED_MATRIX_HEIGHT; i++) {
            pio_sm_put_blocking(pio, sm, leds[i] << 8u);
        }

        // Posição inicial do cursor
        int x = LED_MATRIX_WIDTH / 2;
        int y = LED_MATRIX_HEIGHT / 2;

        int player_sequence[SEQUENCE_MAX_LENGTH][2]; // Armazena a sequência do jogador
        int player_step = 0;                         // Passo atual do jogador
        bool resetGame = false;                      // Flag para reiniciar o jogo

        while (player_step < sequence_length) {
            // Verifica se o botão B foi pressionado
            if (!gpio_get(BUTTON_B_PIN)) {
                clear_leds(leds); // Apaga todos os LEDs
                for (int i = 0; i < LED_MATRIX_WIDTH * LED_MATRIX_HEIGHT; i++) {
                    pio_sm_put_blocking(pio, sm, leds[i] << 8u);
                }
                display_message(&ssd, "Jogo", "Encerrado");
                
                // Aguarda o botão B ser solto
                while (!gpio_get(BUTTON_B_PIN)) {
                    sleep_ms(10);
                }
                // Aguarda o botão B ser pressionado novamente para iniciar um novo jogo
                while (gpio_get(BUTTON_B_PIN)) {
                    sleep_ms(10);
                }
                // Aguarda o botão B ser solto novamente
                while (!gpio_get(BUTTON_B_PIN)) {
                    sleep_ms(10);
                }
                
                // Reinicia o jogo: volta para a fase 1
                sequence_length = 2;
                level = 1;
                display_level(&ssd, level);
                resetGame = true;
                break;
            }

            // Lê os valores do joystick
            uint16_t adc_x = read_adc(27);
            uint16_t adc_y = read_adc(26);

            // Move o cursor
            if (adc_x < 2048 - JOYSTICK_THRESHOLD) {
                x += MOVEMENT_SPEED; // Move para a direita (invertido)
            } else if (adc_x > 2048 + JOYSTICK_THRESHOLD) {
                x -= MOVEMENT_SPEED; // Move para a esquerda (invertido)
            }

            if (adc_y < 2048 - JOYSTICK_THRESHOLD) {
                y -= MOVEMENT_SPEED; // Move para cima
            } else if (adc_y > 2048 + JOYSTICK_THRESHOLD) {
                y += MOVEMENT_SPEED; // Move para baixo
            }

            // Limita a posição do cursor aos limites da matriz
            if (x < 0) x = 0;
            if (x >= LED_MATRIX_WIDTH) x = LED_MATRIX_WIDTH - 1;
            if (y < 0) y = 0;
            if (y >= LED_MATRIX_HEIGHT) y = LED_MATRIX_HEIGHT - 1;

            // Exibe o cursor (usando a cor branca)
            clear_leds(leds);
            set_led_color(leds, x, y, 0xFFFFFF); // Cursor branco
            for (int i = 0; i < LED_MATRIX_WIDTH * LED_MATRIX_HEIGHT; i++) {
                pio_sm_put_blocking(pio, sm, leds[i] << 8u);
            }

            // Verifica se o botão A foi pressionado
            if (!gpio_get(BUTTON_A_PIN)) {
                player_sequence[player_step][0] = x;
                player_sequence[player_step][1] = y;
                player_step++;

                // Aguarda o botão ser solto
                while (!gpio_get(BUTTON_A_PIN)) {
                    sleep_ms(10);
                }
            }

            sleep_ms(50);
        }

        // Se o jogo foi reiniciado via botão B, pula a verificação da sequência
        if (resetGame) {
            continue;
        }

        // Verifica se a sequência do jogador está correta
        bool correct = true;
        for (int i = 0; i < sequence_length; i++) {
            if (player_sequence[i][0] != sequence[i][0] || player_sequence[i][1] != sequence[i][1]) {
                correct = false;
                break;
            }
        }

        // Resposta do jogo
        if (correct) {
            gpio_put(GREEN_LED_PIN, 1); // Acende o LED verde
            display_message(&ssd, "Correto", "Proxima fase");
        
            // Toca o som de vitória: Sequência ascendente (262 Hz → 330 Hz → 392 Hz)
            tocarSom(262, 300);  // Nota Dó (262 Hz) por 300 ms
            sleep_ms(50);         // Pequena pausa entre os sons
            tocarSom(330, 300);  // Nota Mi (330 Hz) por 300 ms
            sleep_ms(50);         // Pequena pausa entre os sons
            tocarSom(392, 400);  // Nota Sol (392 Hz) por 400 ms
        
            sleep_ms(1000);       // Aguarda 1 segundo antes de continuar
            gpio_put(GREEN_LED_PIN, 0);
            sequence_length++;    // Aumenta a dificuldade
            level++;              // Aumenta o nível
            display_level(&ssd, level); // Atualiza a fase no display
        } else {
            gpio_put(RED_LED_PIN, 1); // Acende o LED vermelho
            display_message(&ssd, "Errado", "Voltando para                    fase 1");
        
            // Toca o som de erro: 300 Hz por 200 ms, pausa de 50 ms, 200 Hz por 300 ms
            tocarSom(300, 200);  // Primeiro tom (300 Hz por 200 ms)
            sleep_ms(50);         // Pequena pausa entre os sons
            tocarSom(200, 300);  // Segundo tom (200 Hz por 300 ms)
        
            sleep_ms(1000);       // Aguarda 1 segundo antes de reiniciar
            gpio_put(RED_LED_PIN, 0);
            sequence_length = 2;  // Reinicia o jogo
            level = 1;            // Volta para a fase 1
            display_level(&ssd, level); // Atualiza a fase no display
        }
    }
    return 0;
}
