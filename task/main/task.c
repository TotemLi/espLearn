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
        if (xQueueSend(queue, &i, 0)==pdPASS) {
            ESP_LOGI(TAG, "send success");
        } else {
            ESP_LOGI(TAG, "send fail");
        }
        i++;
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

void taskConsume(void *param) {
    QueueHandle_t queue = (QueueHandle_t)param;

    while (1)
    {
        int data;
        if (xQueueReceive(queue, &data, 0)==pdPASS) {
            ESP_LOGI(TAG, "receve data: %d", data);
        }
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    // TaskHandle_t myTaskA;
    // xTaskCreatePinnedToCore(taskA, "taskA", 2048, NULL, 1, &myTaskA, 1);
    // vTaskDelay(pdMS_TO_TICKS(5*500));
    // vTaskDelete(myTaskA);
    QueueHandle_t queue = xQueueCreate(10, sizeof(int));
    if (queue==NULL) {
        ESP_LOGI(TAG, "create queue err");
        return;
    }
    xTaskCreate(taskProducer, "producer", 2048, queue, 1, NULL);
    xTaskCreate(taskConsume, "consume", 2048, queue, 1, NULL);
}
