# Projeto Joystick com LED RGB e Display OLED

Este projeto é uma atividade prática da residência do CEPEDI de Sistemas Embarcados, Embarcatech. O objetivo é controlar LEDs RGB através de um joystick analógico e exibir a posição do joystick em um display OLED.

## Autor
Mychael Antonio Matos Dos Santos

## Video com demostração do projeto

Link: https://youtu.be/hNsNDYC3Bu8

## Descrição

O projeto utiliza um microcontrolador RP2040 para:
- Ler valores analógicos do joystick (eixos X e Y) através do ADC
- Controlar LEDs RGB via PWM baseado na posição do joystick
- Exibir um quadrado móvel no display OLED que acompanha o movimento do joystick
- Implementar funcionalidades extras através de botões

### Funcionalidades

1. **Controle dos LEDs RGB:**
   - LED Azul (GPIO 12): Intensidade controlada pelo eixo Y do joystick
   - LED Vermelho (GPIO 13): Intensidade controlada pelo eixo X do joystick
   - LED Verde (GPIO 11): Alterna entre ligado/desligado com o botão do joystick

2. **Display OLED:**
   - Exibe um quadrado 8x8 que se move conforme a posição do joystick
   - Possui diferentes estilos de borda que alternam com o botão do joystick

3. **Botões:**
   - Botão do Joystick: Alterna o LED verde e o estilo da borda do display
   - Botão A: Ativa/desativa os LEDs PWM
   - Botão B: Entra no modo BOOTSEL

## Hardware Utilizado

- Placa BitDogLab com RP2040
- Display OLED SSD1306 (128x64)
- Joystick Analógico
- LEDs RGB
- Botões

### Pinagem

- **Joystick:**
  - Eixo X: GPIO 26 (ADC0)
  - Eixo Y: GPIO 27 (ADC1)
  - Botão: GPIO 22

- **LEDs RGB:**
  - Vermelho: GPIO 13
  - Verde: GPIO 11
  - Azul: GPIO 12

- **Display OLED:**
  - SDA: GPIO 14
  - SCL: GPIO 15

- **Botões:**
  - Botão A: GPIO 5
  - Botão B: GPIO 6

## Como Compilar e Executar

1. Clone o repositório
2. Abra a pasta do projeto no VS Code
3. Certifique-se de ter a extensão "Raspberry Pi Pico" instalada no VS Code
4. Pressione F7 para compilar ou selecione "Build" no menu do VS Code
5. Conecte a placa em modo bootloader (pressione BOOTSEL enquanto conecta o USB)
6. Clique em "Upload" ou pressione F5 para carregar o programa na placa

## Requisitos de Software

- Visual Studio Code
- Extensão "Raspberry Pi Pico" para VS Code
- Raspberry Pi Pico SDK (instalado automaticamente pela extensão)

## Licença

Este projeto está sob a licença MIT.

---

Residência do CEPEDI de Sistemas Embarcados, Embarcatech.
