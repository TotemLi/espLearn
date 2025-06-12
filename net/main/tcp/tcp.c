#include <arpa/inet.h>
#include <sys/socket.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <errno.h>
#include "esp_netif.h"
#include "esp_log.h"

static const char *TAG = "tcp";
static int sock;

void receve_msg(void *pvParameters)
{
    char rx_buffer[2048];
    while (1)
    {
        // TODO 解决粘包问题
        int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if (len < 0)
        {
            ESP_LOGE(TAG, "tcp_client recv failed errno: %d", errno);
            return;
        }
        rx_buffer[len] = 0;
        ESP_LOGI(TAG, "recv: %s", rx_buffer);
    }
}

void tcp_client(char *ip, uint16_t port)
{
    struct sockaddr_in dest_addr;
    inet_pton(AF_INET, ip, &dest_addr.sin_addr);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sock < 0)
    {
        ESP_LOGE(TAG, "tcp_client socket sock: %d", sock);
        return;
    }

    int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0)
    {
        ESP_LOGE(TAG, "tcp_client connect errno: %d", errno);
        return;
    }

    xTaskCreate(&receve_msg, "receve_msg", 8192, NULL, 5, NULL);
}

int send_msg(char *payload)
{
    int err = send(sock, payload, strlen(payload), 0);
    return err;
}