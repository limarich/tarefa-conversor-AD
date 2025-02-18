# Documentação do Código

## Introdução
Este código implementa um sistema de controle baseado em joystick utilizando um Raspberry Pi Pico. Ele permite o controle de LEDs via PWM e exibe informações em um display OLED via I2C.

## Dependências
- Biblioteca `pico/stdlib.h` para funções básicas de entrada e saída.
- Biblioteca `hardware/pwm.h` para controle PWM.
- Biblioteca `hardware/adc.h` para leitura do joystick via ADC.
- Biblioteca `hardware/i2c.h` para comunicação com o display OLED.
- Biblioteca `ssd1306.h` para controle do display OLED.
- Biblioteca `font.h` para renderização de texto no display.

## Definições de Hardware
### Pinos Utilizados
- **LEDs:**
  - Vermelho: GPIO 13
  - Azul: GPIO 12
  - Verde: GPIO 11
- **Joystick:**
  - Eixo X: GPIO 27 (ADC1)
  - Eixo Y: GPIO 26 (ADC0)
  - Botão: GPIO 22
- **I2C:**
  - SDA: GPIO 14
  - SCL: GPIO 15
  - Endereço I2C do display: 0x3C
- **Botão extra:**
  - GPIO 5 (BUTTON_A)

## Funcionalidades
1. **Configuração de PWM** (`setup_pwm`)
   - Configura o PWM para os LEDs azul e vermelho.
   - Define a frequência e o nível inicial do PWM.

2. **Configuração da Interface I2C** (`setup_i2c`)
   - Inicializa a comunicação I2C.
   - Configura o display OLED para exibir informações.

3. **Configuração dos botões** (`setup_button`)
   - Inicializa os pinos dos botões e ativa pull-ups internos.

4. **Leitura do Joystick** (`get_joystick_position`)
   - Lê os valores analógicos dos eixos X e Y do joystick.

5. **Controle dos LEDs via Joystick** (`update_leds_with_joystick`)
   - Ajusta a intensidade dos LEDs vermelho e azul com base na posição do joystick.
   - Aplica uma zona morta para evitar pequenas variações involuntárias.

6. **Exibição no Display**
   - Diferentes estilos de bordas:
     - `random_border`: Gera uma borda aleatória.
     - `regular_border`: Gera uma borda fixa.
     - `cool_border`: Anima uma borda pulsante.
     - `none_border`: Remove a borda.
   - `draw_display`: Exibe a borda e desenha um quadrado no display baseado na posição do joystick.

7. **Gerenciamento de Interrupções** (`handle_button_irq`)
   - Alterna entre diferentes estilos de borda ao pressionar o botão do joystick.
   - Liga/desliga o controle PWM dos LEDs ao pressionar `BUTTON_A`.

## Loop Principal (`main`)
1. Inicializa os periféricos (ADC, PWM, I2C, botões, display OLED).
2. Lê a posição inicial do joystick para calibração.
3. Entra em um loop infinito:
   - Lê os valores do joystick.
   - Normaliza os valores entre 0 e 1.
   - Ajusta os LEDs com base na entrada do joystick.
   - Atualiza a exibição do display OLED.
   - Aguarda 30ms antes da próxima iteração.

# Link para youtube:
https://youtu.be/NPsTL46-hC4?si=0PoIVnHL9X3sAfrC
