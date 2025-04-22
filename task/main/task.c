#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "task";

void taskA(void *param) {
    while (1) {
        ESP_LOGI(TAG, "this is taskA");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void app_main(void)
{
    TaskHandle_t myTaskA;
    xTaskCreatePinnedToCore(taskA, "taskA", 2048, NULL, 1, &myTaskA, 1);
    vTaskDelay(pdMS_TO_TICKS(5*500));
    vTaskDelete(myTaskA);
}
