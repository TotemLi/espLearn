#include <stdio.h>
#include <string.h>
#include "driver/uart.h"
#include "esp_log.h"

const uart_port_t uart_num = UART_NUM_2;

void uart_init(void)
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE, // 奇偶校验
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));

    ESP_ERROR_CHECK(uart_set_pin(uart_num, 4, 5, 18, 19));

    const int uart_buffer_size = 2048;
    QueueHandle_t uart_queue;
    ESP_ERROR_CHECK(uart_driver_install(uart_num, uart_buffer_size, uart_buffer_size, 10, &uart_queue, 0));
}

void send_data(char *data)
{
    uart_write_bytes(uart_num, data, strlen(data));
}

char *receve_data()
{
    uint8_t data[128];
    int len = 0;
    ESP_ERROR_CHECK(uart_get_buffered_data_len(uart_num, (size_t *)&len));
    if (len == 0)
    {
        return NULL;
    }
    len = uart_read_bytes(uart_num, data, len, pdMS_TO_TICKS(1000));
    char *buffer = (char *)malloc(len + 1);
    for (int i = 0; i < len; i++)
    {
        buffer[i] = (char)data[i];
    }
    buffer[len] = '\0';
    return buffer;
}

void app_main(void)
{
    char *txt = "hello totemli";
    char *data;
    uart_init();
    send_data(txt);
    while (1)
    {
        data = receve_data();
        if (data == NULL)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }
        ESP_LOGI("uart", "data: %s", data);
        send_data(data);
        free(data);
    }
}
