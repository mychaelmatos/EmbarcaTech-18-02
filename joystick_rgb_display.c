/*
 * Por: Mychael Antonio Matos Dos Santos
 *
 * Projeto: Controle de LED RGB via PWM, Movimentação de quadrado com joystick e exibição no display SSD1306
 *
 * Objetivos:
 * - Ler valores analógicos do joystick (eixos X e Y) através do ADC.
 * - Controlar o LED Vermelho e LED Azul via PWM com base nos valores do joystick.
 * - Controlar o LED Verde via interrupção (toggle a cada acionamento do botão do joystick).
 * - Exibir no display SSD1306 um quadrado (8x8 pixels) que se move proporcionalmente aos valores do joystick.
 * - Alternar o estilo da borda do display ao pressionar o botão do joystick.
 * - Alternar a ativação dos LEDs PWM ao pressionar o botão A.
 * - Usar interrupções com debouncing para os botões.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include "pico/bootrom.h"
#include "pico/time.h"
#include "lib/ssd1306.h"
#include "lib/font.h"

// Definições de pinos e configurações
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define DISPLAY_ADDR 0x3C

// Pinos do Joystick e Botões
#define JOYSTICK_X_PIN 26  // ADC canal 0
#define JOYSTICK_Y_PIN 27  // ADC canal 1
#define JOYSTICK_PB 22     // Botão do Joystick
#define BOTAO_A 5          // Botão A
#define BOTAO_B 6          // Botão B (utilizado para BOOTSEL)

// Pinos do LED RGB
#define LED_VERMELHO 13
#define LED_VERDE    11
#define LED_AZUL     12

// Configurações de PWM
#define PWM_WRAP 4095

// Configuração de debounce (em milissegundos)
#define DEBOUNCE_DELAY 200

// Número de estilos de borda
#define NUM_BORDER_STYLES 2

// Variáveis globais voláteis para controle via interrupção
volatile uint32_t last_debounce_joystick = 0;
volatile uint32_t last_debounce_botaoA = 0;
volatile bool toggle_pwm = true;       // Estado de ativação dos LEDs PWM
volatile bool led_verde_on = false;    // Estado do LED Verde
volatile uint8_t border_style = 0;     // Estilo da borda do display

// Rotina de tratamento de interrupção (IRQ) para os botões
void gpio_irq_handler(uint gpio, uint32_t events) {
    // Botão B: ativa modo BOOTSEL
    if (gpio == BOTAO_B && (events & GPIO_IRQ_EDGE_FALL)) {
        reset_usb_boot(0, 0);
        return;
    }
    
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    // Botão do Joystick: alterna LED Verde e estilo da borda
    if (gpio == JOYSTICK_PB && (events & GPIO_IRQ_EDGE_FALL)) {
        if (current_time - last_debounce_joystick < DEBOUNCE_DELAY)
            return;
        last_debounce_joystick = current_time;
        
        led_verde_on = !led_verde_on;
        border_style = (border_style + 1) % NUM_BORDER_STYLES;
    }
    
    // Botão A: alterna ativação dos LEDs PWM
    if (gpio == BOTAO_A && (events & GPIO_IRQ_EDGE_FALL)) {
        if (current_time - last_debounce_botaoA < DEBOUNCE_DELAY)
            return;
        last_debounce_botaoA = current_time;
        
        toggle_pwm = !toggle_pwm;
    }
}

int main() {
    // Inicialização padrão
    stdio_init_all();
    
    // Configuração do pino BOTAO_B para modo BOOTSEL
    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);
    // Registra a rotina de interrupção com callback global
    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    
    // Configura o botão do Joystick
    gpio_init(JOYSTICK_PB);
    gpio_set_dir(JOYSTICK_PB, GPIO_IN);
    gpio_pull_up(JOYSTICK_PB);
    gpio_set_irq_enabled(JOYSTICK_PB, GPIO_IRQ_EDGE_FALL, true);
    
    // Configura o Botão A
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);
    gpio_set_irq_enabled(BOTAO_A, GPIO_IRQ_EDGE_FALL, true);
    
    // Inicialização do I2C para o display SSD1306 (400kHz)
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    
    // Inicializa e configura o display SSD1306
    ssd1306_t ssd;
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, DISPLAY_ADDR, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    
    // Limpa o display
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
    
    // Inicializa o ADC e configura os pinos do Joystick para entrada analógica
    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);
    
    // Configura os pinos dos LEDs RGB para PWM
    // LED Vermelho
    gpio_set_function(LED_VERMELHO, GPIO_FUNC_PWM);
    uint slice_red = pwm_gpio_to_slice_num(LED_VERMELHO);
    pwm_set_wrap(slice_red, PWM_WRAP);
    pwm_set_gpio_level(LED_VERMELHO, 0);
    pwm_set_enabled(slice_red, true);
    
    // LED Verde
    gpio_set_function(LED_VERDE, GPIO_FUNC_PWM);
    uint slice_green = pwm_gpio_to_slice_num(LED_VERDE);
    pwm_set_wrap(slice_green, PWM_WRAP);
    pwm_set_gpio_level(LED_VERDE, 0);
    pwm_set_enabled(slice_green, true);
    
    // LED Azul
    gpio_set_function(LED_AZUL, GPIO_FUNC_PWM);
    uint slice_blue = pwm_gpio_to_slice_num(LED_AZUL);
    pwm_set_wrap(slice_blue, PWM_WRAP);
    pwm_set_gpio_level(LED_AZUL, 0);
    pwm_set_enabled(slice_blue, true);
    
    // Variáveis auxiliares para os valores do ADC
    uint16_t adc_value_x, adc_value_y;
    // Variáveis para posicionamento do quadrado no display
    uint8_t pos_x, pos_y;
    
    while (true) {
        // Leitura do eixo X do joystick (ADC canal 0)
        adc_select_input(0);
        adc_value_x = adc_read();
        // Leitura do eixo Y do joystick (ADC canal 1)
        adc_select_input(1);
        adc_value_y = adc_read();
        
        // Cálculo das intensidade PWM para LED Vermelho (eixo X) e LED Azul (eixo Y)
        uint16_t pwm_red = 0;
        uint16_t pwm_blue = 0;
        
        // Aumentando a zona morta no centro do joystick (tolerância de ±300 em torno do centro)
        const uint16_t DEADZONE = 300;
        
        // Cálculo PWM LED Vermelho (eixo X)
        if (abs(adc_value_x - 2048) > DEADZONE) {
            if (adc_value_x < (2048 - DEADZONE)) {
                // Movimento para esquerda
                pwm_red = ((2048 - DEADZONE - adc_value_x) * PWM_WRAP) / 2048;
            } else if (adc_value_x > (2048 + DEADZONE)) {
                // Movimento para direita
                pwm_red = ((adc_value_x - (2048 + DEADZONE)) * PWM_WRAP) / 2048;
            }
        }
        
        // Cálculo PWM LED Azul (eixo Y)
        if (abs(adc_value_y - 2048) > DEADZONE) {
            if (adc_value_y < (2048 - DEADZONE)) {
                // Movimento para cima
                pwm_blue = ((2048 - DEADZONE - adc_value_y) * PWM_WRAP) / 2048;
            } else if (adc_value_y > (2048 + DEADZONE)) {
                // Movimento para baixo
                pwm_blue = ((adc_value_y - (2048 + DEADZONE)) * PWM_WRAP) / 2048;
            }
        }
        
        uint16_t pwm_green = led_verde_on ? PWM_WRAP : 0;
        
        // Se PWM estiver desativado, força os níveis para 0
        if (!toggle_pwm) {
            pwm_red = 0;
            pwm_blue = 0;
            pwm_green = 0;
        }
        
        // Atualiza os níveis de PWM dos LEDs
        pwm_set_gpio_level(LED_VERMELHO, pwm_red);
        pwm_set_gpio_level(LED_AZUL, pwm_blue);
        pwm_set_gpio_level(LED_VERDE, pwm_green);

        // Corrigindo o mapeamento dos valores do joystick para a posição do quadrado no display
        pos_x = (adc_value_y * (WIDTH - 8)) / 4095;          // Y mapeia para X (sem inversão)
        pos_y = ((4095 - adc_value_x) * (HEIGHT - 8)) / 4095;  // Inverte X e mapeia para Y
        
        // Atualiza o conteúdo do display
        // Limpa o display
        ssd1306_fill(&ssd, false);
        
        // Desenha a borda do display de acordo com o estilo selecionado
        if (border_style == 0) {
            // Borda simples: retângulo ao redor do display
            ssd1306_rect(&ssd, 0, 0, WIDTH, HEIGHT, true, false);
        } else if (border_style == 1) {
            // Borda dupla: desenha dois retângulos concêntricos
            ssd1306_rect(&ssd, 0, 0, WIDTH, HEIGHT, true, false);
            ssd1306_rect(&ssd, 2, 2, WIDTH - 4, HEIGHT - 4, true, false);
        }
        
        // Desenha o quadrado móvel que representa a posição do joystick
        ssd1306_rect(&ssd, pos_y, pos_x, 8, 8, true, true);
        
        // Atualiza o display
        ssd1306_send_data(&ssd);
        
        sleep_ms(100);
    }
    
    return 0;
}