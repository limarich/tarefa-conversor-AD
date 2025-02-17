#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"

#define LED_RED 13
#define LED_BLUE 12
#define LED_GREEN 11
#define WRAP 2000
#define VX 27
#define VY 26

#define DEAD_ZONE 40

void pwm_setup(uint gpio)
{
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(gpio);
    pwm_set_clkdiv(slice, 2.0);
    pwm_set_wrap(slice, WRAP);
    pwm_set_gpio_level(gpio, 0);
    pwm_set_enabled(slice, true);
}

int main()
{
    stdio_init_all();
    // ADC
    adc_init();
    adc_gpio_init(VY);
    adc_gpio_init(VX);
    // PWM
    pwm_setup(LED_RED);
    pwm_setup(LED_BLUE);

    adc_select_input(0);
    uint center_y = adc_read();
    adc_select_input(1);
    uint center_x = adc_read();

    while (true)
    {
        uint vy_pwm = 0, vx_pwm = 0;

        // leitura do VY
        adc_select_input(0);
        uint vy = adc_read();

        if (vy > center_y + DEAD_ZONE)
        {
            vy_pwm = ((vy - center_y) * WRAP) / (4095 - center_y);
        }
        else if (vy < center_y - DEAD_ZONE)
        {
            vy_pwm = ((center_y - vy) * WRAP) / center_y;
        }
        pwm_set_gpio_level(LED_BLUE, vy_pwm);

        // leitura do VX
        adc_select_input(1);
        uint vx = adc_read();

        if (vx > center_x + DEAD_ZONE)
        {
            vx_pwm = ((vx - center_x) * WRAP) / (4095 - center_x);
        }
        else if (vx < center_x - DEAD_ZONE)
        {
            vx_pwm = ((center_x - vx) * WRAP) / center_x;
        }
        pwm_set_gpio_level(LED_RED, vx_pwm);

        sleep_ms(40);
    }
}
