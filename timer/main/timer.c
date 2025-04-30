#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gptimer.h"
#include "freertos/semphr.h"

SemaphoreHandle_t semaphoreHandle;

void do_task(void *param)
{
    while (1)
    {
        if (xSemaphoreTake(semaphoreHandle, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI("time", "alarm");
        }
    }
}

static bool time_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(semaphoreHandle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    return true;
}

void timer_init()
{
    // 新建定时器
    gptimer_config_t config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1MHZ
    };
    gptimer_handle_t timer;
    gptimer_new_timer(&config, &timer);

    // 配置定时
    gptimer_alarm_config_t alarm_config = {
        .alarm_count = 3000000,
        .reload_count = 0,
        .flags.auto_reload_on_alarm = 1,
    };
    gptimer_set_alarm_action(timer, &alarm_config);

    // 设置定时到了执行的回调
    gptimer_event_callbacks_t cbs = {
        .on_alarm = time_cb,
    };
    gptimer_register_event_callbacks(timer, &cbs, NULL);

    // 使能定时器
    gptimer_enable(timer);

    // 启动定时器
    gptimer_start(timer);
}

void app_main(void)
{
    semaphoreHandle = xSemaphoreCreateBinary();
    xTaskCreate(do_task, "do_task", 2048, NULL, 5, NULL);
    timer_init();
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
