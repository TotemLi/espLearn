#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

// void led_init()
// {
//     gpio_config_t cfg = {
//         //     uint64_t pin_bit_mask;          /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
//         // gpio_mode_t mode;               /*!< GPIO mode: set input/output mode                     */
//         // gpio_pullup_t pull_up_en;       /*!< GPIO pull-up                                         */
//         // gpio_pulldown_t pull_down_en;   /*!< GPIO pull-down                                       */
//         // gpio_int_type_t intr_type;      /*!< GPIO interrupt type
//         .pin_bit_mask = GPIO_NUM_38,
//         .mode = GPIO_MODE_OUTPUT,
//         .pull_up_en = GPIO_PULLUP_ENABLE,
//         .pull_down_en = GPIO_PULLDOWN_ENABLE,
//     };
//     gpio_config(&cfg);
// }

// void app_main(void)
// {
//     while (1)
//     {
//         ESP_LOGI("debug", "1111");
//         gpio_set_level(GPIO_NUM_38, !gpio_get_level(GPIO_NUM_38));
//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }
// }

#define RGB_LED_R_PIN GPIO_NUM_38
#define RGB_LED_G_PIN GPIO_NUM_39
#define RGB_LED_B_PIN GPIO_NUM_40

static const char *TAG = "RGB_LED";

void app_main(void)
{
    // 配置RGB LED引脚为输出模式
    gpio_reset_pin(RGB_LED_R_PIN);
    gpio_set_direction(RGB_LED_R_PIN, GPIO_MODE_OUTPUT);
    gpio_reset_pin(RGB_LED_G_PIN);
    gpio_set_direction(RGB_LED_G_PIN, GPIO_MODE_OUTPUT);
    gpio_reset_pin(RGB_LED_B_PIN);
    gpio_set_direction(RGB_LED_B_PIN, GPIO_MODE_OUTPUT);

    while (1)
    {
        // 点亮红色LED
        ESP_LOGI(TAG, "Turning on red LED");
        gpio_set_level(RGB_LED_R_PIN, 1);
        gpio_set_level(RGB_LED_G_PIN, 0);
        gpio_set_level(RGB_LED_B_PIN, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        // 点亮绿色LED
        ESP_LOGI(TAG, "Turning on green LED");
        gpio_set_level(RGB_LED_R_PIN, 0);
        gpio_set_level(RGB_LED_G_PIN, 1);
        gpio_set_level(RGB_LED_B_PIN, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        // 点亮蓝色LED
        ESP_LOGI(TAG, "Turning on blue LED");
        gpio_set_level(RGB_LED_R_PIN, 0);
        gpio_set_level(RGB_LED_G_PIN, 0);
        gpio_set_level(RGB_LED_B_PIN, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}