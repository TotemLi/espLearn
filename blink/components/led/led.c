#include <stdio.h>
#include "led.h"
#include "driver/gpio.h"

#define BLINK_GPIO GPIO_NUM_12

void blink_led(uint8_t state)
{
    gpio_set_level(BLINK_GPIO, state);
}

void configure_led(void) {
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}