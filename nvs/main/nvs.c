#include <stdio.h>
#include <inttypes.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

static char *TAG = "nvs";

typedef struct
{
    char ssid[50];
    char password[50];
} ap_t;

void app_main(void)
{
    esp_err_t err = nvs_flash_init_partition("my_nvs");
    ESP_ERROR_CHECK(err);
    ESP_LOGI(TAG, "nvs init success");

    nvs_handle_t my_handle;
    err = nvs_open("totemli", NVS_READWRITE, &my_handle);
    ESP_ERROR_CHECK(err);

    err = nvs_set_i32(my_handle, "age", 29);
    ESP_ERROR_CHECK(err);

    err = nvs_set_str(my_handle, "name", "totemli");
    ESP_ERROR_CHECK(err);

    ap_t ap_data = {
        .ssid = "totemli",
        .password = "totemli",
    };
    err = nvs_set_blob(my_handle, "ap", &ap_data, sizeof ap_data);
    ESP_ERROR_CHECK(err);

    err = nvs_commit(my_handle);
    ESP_ERROR_CHECK(err);

    int32_t age;
    nvs_get_i32(my_handle, "age", &age);
    ESP_LOGI(TAG, "age: %" PRId32, age);

    size_t required_size;
    nvs_get_str(my_handle, "name", NULL, &required_size);
    char *name = (char *)malloc(required_size);
    nvs_get_str(my_handle, "name", name, &required_size);
    ESP_LOGI(TAG, "name: %s, len: %d", name, required_size);

    ap_t blob_data;
    size_t length = sizeof(blob_data);
    nvs_get_blob(my_handle, "ap", &blob_data, &length);
    ESP_LOGI(TAG, "ssid: %s, password: %s", blob_data.ssid, blob_data.password);

    nvs_close(my_handle);
}
