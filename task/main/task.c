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

void taskProducer(void *param) {
    QueueHandle_t queue = (QueueHandle_t)param;

    int i = 0;
    while (1)
    {
        if (xQueueSend(queue, &i, 0)!=pdPASS) {
            ESP_LOGI(TAG, "send success");
        }
    }
    
}

void app_main(void)
{
    TaskHandle_t myTaskA;
    xTaskCreatePinnedToCore(taskA, "taskA", 2048, NULL, 1, &myTaskA, 1);
    vTaskDelay(pdMS_TO_TICKS(5*500));
    vTaskDelete(myTaskA);
}
