#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"
#include "inc/font.h"

#define LED_RED 13
#define LED_BLUE 12
#define LED_GREEN 11
#define PWM_WRAP 2000
#define JOYSTICK_X_PIN 27
#define JOYSTICK_Y_PIN 26
#define JOYSTICK_BUTTON 22
#define I2C_PORT i2c1
#define I2C_SDA_PIN 14
#define I2C_SCL_PIN 15
#define I2C_ADDRESS 0x3C
#define DEAD_ZONE 40
#define BUTTON_A 5

ssd1306_t ssd;
uint border_style = 0;
uint last_interrupt = 0;
bool is_pwm_active = true;

void setup_pwm(uint gpio)
{
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(gpio);
    pwm_set_clkdiv(slice, 2.0);
    pwm_set_wrap(slice, PWM_WRAP);
    pwm_set_gpio_level(gpio, 0);
    pwm_set_enabled(slice, true);
}

void setup_i2c()
{
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, I2C_ADDRESS, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
}
void setup_button(uint gpio)
{
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_IN);
    gpio_pull_up(gpio);
}

void get_joystick_position(uint *joystick_x, uint *joystick_y)
{
    adc_select_input(0);
    *joystick_y = adc_read();
    adc_select_input(1);
    *joystick_x = adc_read();
}

void update_leds_with_joystick(uint *joystick_x, uint *joystick_y, uint joystick_x_center, uint joystick_y_center)
{
    uint pwm_y = 0, pwm_x = 0;
    if (*joystick_y > joystick_y_center + DEAD_ZONE)
    {
        pwm_y = ((*joystick_y - joystick_y_center) * PWM_WRAP) / (4095 - joystick_y_center);
    }
    else if (*joystick_y < joystick_y_center - DEAD_ZONE)
    {
        pwm_y = ((joystick_y_center - *joystick_y) * PWM_WRAP) / joystick_y_center;
    }
    pwm_set_gpio_level(LED_BLUE, pwm_y);

    if (*joystick_x > joystick_x_center + DEAD_ZONE)
    {
        pwm_x = ((*joystick_x - joystick_x_center) * PWM_WRAP) / (4095 - joystick_x_center);
    }
    else if (*joystick_x < joystick_x_center - DEAD_ZONE)
    {
        pwm_x = ((joystick_x_center - *joystick_x) * PWM_WRAP) / joystick_x_center;
    }
    pwm_set_gpio_level(LED_RED, pwm_x);
}

void random_border()
{
    for (uint x = 0; x < WIDTH; x++)
    {
        bool state = rand() % 2;

        ssd1306_pixel(&ssd, x, 0, state);
        ssd1306_pixel(&ssd, x, HEIGHT - 1, state);
    }

    for (uint y = 0; y < HEIGHT; y++)
    {
        bool state = rand() % 2;

        ssd1306_pixel(&ssd, 0, y, state);
        ssd1306_pixel(&ssd, WIDTH - 1, y, state);
    }
}

void regular_border()
{
    for (uint x = 0; x < WIDTH; x++)
    {
        ssd1306_pixel(&ssd, x, 0, 1);
        ssd1306_pixel(&ssd, x, HEIGHT - 1, 1);
    }

    for (uint y = 0; y < HEIGHT; y++)
    {

        ssd1306_pixel(&ssd, 0, y, 1);
        ssd1306_pixel(&ssd, WIDTH - 1, y, 1);
    }
}
void none_border()
{
    for (uint x = 0; x < WIDTH; x++)
    {
        ssd1306_pixel(&ssd, x, 0, 0);
        ssd1306_pixel(&ssd, x, HEIGHT - 1, 0);
    }

    for (uint y = 0; y < HEIGHT; y++)
    {

        ssd1306_pixel(&ssd, 0, y, 0);
        ssd1306_pixel(&ssd, WIDTH - 1, y, 0);
    }
}
void cool_border()
{
    static uint size = 0;       // Controla o tamanho da expansão
    static bool growing = true; // Alterna entre crescer e diminuir

    ssd1306_fill(&ssd, false);

    for (uint x = size; x < WIDTH - size; x++)
    {
        ssd1306_pixel(&ssd, x, size, true);
        ssd1306_pixel(&ssd, x, HEIGHT - size - 1, true);
    }

    for (uint y = size; y < HEIGHT - size; y++)
    {
        ssd1306_pixel(&ssd, size, y, true);
        ssd1306_pixel(&ssd, WIDTH - size - 1, y, true);
    }

    ssd1306_send_data(&ssd);

    // Alterna entre crescer e diminuir a borda
    if (growing)
    {
        size++;
        if (size >= 5) // Define o limite máximo de expansão
            growing = false;
    }
    else
    {
        size--;
        if (size == 0)
            growing = true;
    }
}

void draw_border(uint display_y, uint display_x)
{

    switch (border_style)
    {
    case 0:
        regular_border();
        break;
    case 1:
        random_border();
        break;
    case 2:
        cool_border();
        break;
    default:
        none_border();
        break;
    }
    ssd1306_rect(&ssd, display_y, display_x, 8, 8, true, false);
    ssd1306_send_data(&ssd);
}

void handle_button_irq(uint gpio, uint32_t e)
{
    uint current_time = to_ms_since_boot(get_absolute_time());
    if (current_time - last_interrupt > 200)
    {
        last_interrupt = current_time;
        if (gpio == JOYSTICK_BUTTON)
        {
            if (border_style > 2)
                border_style = 0;
            else
                border_style++;
            gpio_put(LED_GREEN, !gpio_get(LED_GREEN));
        }
        else if (gpio == BUTTON_A)
        {
            is_pwm_active = !is_pwm_active;
            uint red_slice = pwm_gpio_to_slice_num(LED_RED);
            uint blue_slice = pwm_gpio_to_slice_num(LED_BLUE);
            pwm_set_enabled(red_slice, is_pwm_active);
            pwm_set_enabled(blue_slice, is_pwm_active);
        }
        }
}

int main()
{
    stdio_init_all();
    adc_init();
    adc_gpio_init(JOYSTICK_Y_PIN);
    adc_gpio_init(JOYSTICK_X_PIN);
    setup_pwm(LED_RED);
    setup_pwm(LED_BLUE);

    adc_select_input(0);
    uint joystick_y_center = adc_read();
    adc_select_input(1);
    uint joystick_x_center = adc_read();

    setup_i2c();

    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_put(LED_GREEN, 0);

    setup_button(JOYSTICK_BUTTON);
    setup_button(BUTTON_A);

    gpio_set_irq_enabled_with_callback(JOYSTICK_BUTTON, GPIO_IRQ_EDGE_FALL, true, &handle_button_irq);
    gpio_set_irq_enabled(BUTTON_A, GPIO_IRQ_EDGE_FALL, true);

    while (true)
    {
        uint joystick_x = 0, joystick_y = 0;
        float normalized_x, normalized_y = 0.0;

        get_joystick_position(&joystick_x, &joystick_y);
        normalized_x = joystick_x / 4095.0;
        normalized_y = joystick_y / 4095.0;
        printf("VX:%.2f VY:%.2f \n", normalized_x, normalized_y);

        update_leds_with_joystick(&joystick_x, &joystick_y, joystick_x_center, joystick_y_center);

        int display_x = (uint8_t)(WIDTH * normalized_x);
        int display_y = HEIGHT - (uint8_t)(HEIGHT * normalized_y);

        ssd1306_fill(&ssd, false);

        draw_border(display_y, display_x);

        sleep_ms(30);
    }
}
