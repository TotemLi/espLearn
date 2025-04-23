#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"

static const char *TAG = "task";

void taskA(void *param)
{
    while (1)
    {
        ESP_LOGI(TAG, "this is taskA");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void taskProducer(void *param)
{
    QueueHandle_t queue = (QueueHandle_t)param;

    int i = 0;
    while (1)
    {
        if (xQueueSend(queue, &i, 0) == pdPASS)
        {
            ESP_LOGI(TAG, "send success");
        }
        else
        {
            ESP_LOGI(TAG, "send fail");
        }
        i++;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void taskConsume(void *param)
{
    QueueHandle_t queue = (QueueHandle_t)param;

    while (1)
    {
        int data;
        if (xQueueReceive(queue, &data, 0) == pdPASS)
        {
            ESP_LOGI(TAG, "receve data: %d", data);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void testTask(void)
{
    TaskHandle_t myTaskA;
    xTaskCreatePinnedToCore(taskA, "taskA", 2048, NULL, 1, &myTaskA, 1);
    vTaskDelay(pdMS_TO_TICKS(5 * 500));
    vTaskDelete(myTaskA);
}

void testQueue(void)
{
    QueueHandle_t queue = xQueueCreate(10, sizeof(int));
    if (queue == NULL)
    {
        ESP_LOGI(TAG, "create queue err");
        return;
    }
    xTaskCreate(taskProducer, "producer", 2048, queue, 1, NULL);
    xTaskCreate(taskConsume, "consume", 2048, queue, 1, NULL);
}

void consumeSet(void *param)
{
    QueueSetHandle_t queueSet = (QueueSetHandle_t)param;
    QueueSetMemberHandle_t queue;
    while (1)
    {
        queue = xQueueSelectFromSet(queueSet, 0);
        if (queue != NULL)
        {
            int tmp;
            if (xQueueReceive(queue, &tmp, 0) == pdPASS)
            {
                ESP_LOGI(TAG, "recevie data: %d", tmp);
            }
            else
            {
                ESP_LOGE(TAG, "recevie data fail");
            }
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void testQueueSet(void)
{
    QueueHandle_t queue1, queue2;
    queue1 = xQueueCreate(10, sizeof(int));
    queue2 = xQueueCreate(10, sizeof(int));

    QueueSetHandle_t queueSet = xQueueCreateSet(20);
    xQueueAddToSet(queue1, queueSet);
    xQueueAddToSet(queue2, queueSet);

    if (queue1 != NULL && queue2 != NULL && queueSet != NULL)
    {
        xTaskCreate(taskProducer, "producer1", 2048, queue1, 1, NULL);
        xTaskCreate(taskProducer, "producer2", 2048, queue2, 1, NULL);
        xTaskCreate(consumeSet, "consumeSet", 2048, queueSet, 1, NULL);
    }
}

void consumeSemaphoreB(void *param)
{
    SemaphoreHandle_t semaphoreB = (SemaphoreHandle_t)param;
    while (1)
    {
        xSemaphoreTake(semaphoreB, portMAX_DELAY);
        ESP_LOGI(TAG, "take a semaphore");
        vTaskDelay(pdMS_TO_TICKS(1000));
        xSemaphoreGive(semaphoreB); // 释放信号量
    }
}

void testSemaphore(void)
{
    SemaphoreHandle_t semaphoreB = xSemaphoreCreateBinary();
    xSemaphoreGive(semaphoreB);
    xTaskCreate(consumeSemaphoreB, "consumeSemaphoreB", 2048, semaphoreB, 1, NULL);
}

void app_main(void)
{
    testSemaphore();
}
