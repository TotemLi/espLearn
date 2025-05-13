#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include "tf.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

static const char *TAG = "spi";

#define PIN_NUM_CS 8
#define PIN_NUM_SCLK 2
#define PIN_NUM_MOSI 4
#define PIN_NUM_MISO 5

#define SPI_TRANSFER_MAX_SIZE 4096
#define SPI_HOST_FREQ 4000000UL

void init_tf(void)
{
    esp_err_t err;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5, // 每打开一个文件需要约1KB内存
        .allocation_unit_size = 16 * 1024,
    };

    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    spi_bus_config_t bus_cfg = {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_SCLK,
        .quadhd_io_num = -1,
        .quadwp_io_num = -1,
        .max_transfer_sz = SPI_TRANSFER_MAX_SIZE,
    };
    err = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    ESP_ERROR_CHECK(err);

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    err = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);
    ESP_ERROR_CHECK(err);
    ESP_LOGI(TAG, "mount success!");
}

void list_dir(const char *path)
{
    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        ESP_LOGE(TAG, "open dir: %s err.", path);
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        ESP_LOGI(TAG, "name: %s, type: %d, ino: %d", entry->d_name, entry->d_type, entry->d_ino);
    }
    closedir(dir);
    return;
}

void make_dir(const char *path)
{
    int ret = mkdir(path, 0777);
    if (ret != 0 && errno != EEXIST)
    {
        ESP_LOGE(TAG, "mkdir path: %s err, ret: %d, errno: %d, errmsg: %s", path, ret, errno, strerror(errno));
        return;
    }
}

void write_file(const char *path, const char *data)
{
    FILE *f = fopen(path, "w");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "can't open file: %s, errno: %d", path, errno);
        return;
    }

    size_t written_len = fwrite(data, 1, strlen(data), f);
    if (written_len != strlen(data))
    {
        ESP_LOGE(TAG, "write err, write len: %d", written_len);
        return;
    }
    fclose(f);
}
