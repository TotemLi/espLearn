#include <stdio.h>
#include "esp_system.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_mac.h"

static const char *TAG = "hardware";

void app_main(void)
{
    ESP_LOGE(TAG, "IDF版本号：%s", esp_get_idf_version());

    esp_chip_info_t chipInfo;
    esp_chip_info(&chipInfo);
    ESP_LOGE(TAG, "芯片类型：%d", chipInfo.model);
    ESP_LOGE(TAG, "芯片特征：%d", chipInfo.features);
    ESP_LOGE(TAG, "cpu核心数：%d", chipInfo.cores);
    ESP_LOGE(TAG, "cpu版本号：%d", chipInfo.revision);

    uint32_t size_flash_chip;
    esp_flash_get_size(NULL, &size_flash_chip);
    ESP_LOGE(TAG, "flash容量：%d MB", size_flash_chip/1024/1024);

    uint8_t macAddr;
    esp_efuse_mac_get_default(&macAddr);
    ESP_LOGE(TAG, "mac addr%d MB", macAddr);
}
