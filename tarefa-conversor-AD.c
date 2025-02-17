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
#define I2C_PORT i2c1
#define I2C_SDA_PIN 14
#define I2C_SCL_PIN 15
#define I2C_ADDRESS 0x3C
#define DEAD_ZONE 40

ssd1306_t ssd;

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
        ssd1306_rect(&ssd, display_y, display_x, 8, 8, true, false);
        ssd1306_send_data(&ssd);

        sleep_ms(100);
    }
}
