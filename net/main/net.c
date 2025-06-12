#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wifi/wifi.h"
#include "http/http.h"
#include "nvs_flash.h"

static const char *TAG = "net";

// #define SSID "TP-LINK_A659"
// #define PASSWORD "lichaolong"
// #define SSID "XIAOJING"
// #define PASSWORD "xiaojing"
// #define AUTHMODE WIFI_AUTH_WPA2_PSK

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init();

    vTaskDelay(pdMS_TO_TICKS(10000));

    xTaskCreate(&http_rest_with_url, "http_rest_with_url", 8192, NULL, 5, NULL);

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
