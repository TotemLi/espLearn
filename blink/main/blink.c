#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"

void app_main(void)
{
    uint8_t led_state = 0;

    configure_led();

    while (1)
    {
        led_state = !led_state;
        blink_led(led_state);
        vTaskDelay(1000);
    }
}
