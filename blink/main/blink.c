#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "iot_button.h"
#include "button_gpio.h"

uint8_t led_state = 0;

static void button_single_click_cb(void *arg, void *usr_data) {
    led_state = !led_state;
    blink_led(led_state);
}

void buttopn_init(uint32_t button_num) {
    button_config_t btn_cfg = {0};
    button_gpio_config_t btn_gpio_cfg = {
        .gpio_num = button_num,
        .active_level = 0,  // 低电平有效
    };
    button_handle_t gpio_btn = NULL;
    esp_err_t ret = iot_button_new_gpio_device(&btn_cfg, &btn_gpio_cfg, &gpio_btn);
    if (NULL == gpio_btn || ret!=ESP_OK) {
        // TODO 报错
        return;
    }
    iot_button_register_cb(gpio_btn, BUTTON_SINGLE_CLICK, NULL, button_single_click_cb, NULL);
    return;
}

void app_main(void)
{
    configure_led();
    buttopn_init(18);

    while (1)
    {
        vTaskDelay(1000);
    }
}
