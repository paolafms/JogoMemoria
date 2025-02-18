# Link do vídeo:

# Projeto de Jogo de Memória com Matriz de LEDs e Joystick
Este projeto é um jogo de memória desenvolvido para a plataforma Raspberry Pi Pico W, utilizando uma matriz de LEDs WS2812, um joystick analógico, um display OLED SSD1306, e botões para interação. O objetivo do jogo é memorizar e repetir uma sequência de LEDs que é exibida na matriz.

# Componentes Utilizados:
- Raspberry Pi Pico
- Matriz de LEDs WS2812 (5x5)
- Joystick Analógico
- Display OLED SSD1306
- Botões (A e B)
- LEDs (Vermelho e Verde)
- Buzzer

# Funcionalidades:
1. Exibição de Sequências: O jogo exibe uma sequência de LEDs que o jogador deve memorizar.
2. Controle via Joystick: O jogador usa o joystick para mover um cursor na matriz de LEDs.
3. Repetição da Sequência: O jogador deve repetir a sequência exibida pressionando o botão A nas posições corretas.
4. Feedback Visual e Sonoro: O jogo fornece feedback visual (LEDs verde e vermelho) e sonoro (buzzer) para indicar se a sequência foi repetida corretamente.
5. Níveis de Dificuldade: A cada fase, a sequência aumenta em complexidade, tornando o jogo mais desafiador.
6. Reinicialização do Jogo: O botão B permite reiniciar o jogo a qualquer momento.

# Estrutura do Código:
- Inicialização de Periféricos: Configuração dos GPIOs, ADC, I2C, PWM, e inicialização da matriz de LEDs e do display OLED.
- Geração de Sequências: Função para gerar sequências aleatórias de LEDs.
- Exibição de Sequências: Função para exibir a sequência de LEDs na matriz.
- Leitura do Joystick: Função para ler os valores do joystick e mover o cursor na matriz.
- Verificação da Sequência: Função para verificar se o jogador repetiu a sequência corretamente.
- Feedback Visual e Sonoro: Funções para acender LEDs e tocar sons de vitória ou erro.
- Loop Principal: Controla o fluxo do jogo, incluindo a exibição de sequências, leitura do joystick, e verificação da sequência.

# Autora:
- Paola Fagundes Moreira Santos

