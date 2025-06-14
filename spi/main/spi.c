#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
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

#define MOUNT_POINT "/sdcard"

void tf_init(void)
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

    // sdmmc_card_print_info(stdout, card);
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

void app_main(void)
{
    tf_init();
    // make_dir(MOUNT_POINT "/totemli");
    // write_file(MOUNT_POINT "/totemli/a.txt", "hello world!");
    // list_dir(MOUNT_POINT);
    list_dir(MOUNT_POINT "/totemli");
}

// static spi_device_handle_t spi;

// static void spi_init()
// {
//     spi_bus_config_t busfg = {
//         .miso_io_num = PIN_NUM_MISO,
//         .mosi_io_num = PIN_NUM_MOSI,
//         .sclk_io_num = PIN_NUM_SCLK,
//         .max_transfer_sz = SPI_TRANSFER_MAX_SIZE,
//         .flags = SPICOMMON_BUSFLAG_MASTER,
//     };
//     /*
//     flags 字段说明
//     #define SPICOMMON_BUSFLAG_SLAVE         0          /// 设置设备为从模式
//     #define SPICOMMON_BUSFLAG_MASTER        (1<<0)     /// 设置设备为主模式
//     #define SPICOMMON_BUSFLAG_IOMUX_PINS    (1<<1)     /// 检查是否使用IOMUX最为输入输出
//     #define SPICOMMON_BUSFLAG_GPIO_PINS     (1<<2)     /// 强制使用GPIO Matrix作为输入输出
//     // 如果两个标记都没有设置，那么内部会检查设置的IO引脚编号是否可以通过IOMUX配置，如果可以则使用IOMUX，否则使用GPIO Matrix。
//     #define SPICOMMON_BUSFLAG_SCLK          (1<<3)     /// 检查是否配置了SCLK
//     #define SPICOMMON_BUSFLAG_MISO          (1<<4)     /// 检查是否配置了MISO
//     #define SPICOMMON_BUSFLAG_MOSI          (1<<5)     /// 检查是否配置了MOSI
//     #define SPICOMMON_BUSFLAG_DUAL          (1<<6)     /// 检查是否可以为双输出
//     #define SPICOMMON_BUSFLAG_WPHD          (1<<7)     /// 检查WP and HD pins
//     #define SPICOMMON_BUSFLAG_QUAD          (SPICOMMON_BUSFLAG_DUAL|SPICOMMON_BUSFLAG_WPHD)     /// 检查是否为四线输入模式
//     #define SPICOMMON_BUSFLAG_IO4_IO7       (1<<8)     /// 检查 IO4~IO7 pins
//     #define SPICOMMON_BUSFLAG_OCTAL         (SPICOMMON_BUSFLAG_QUAD|SPICOMMON_BUSFLAG_IO4_IO7)  /// 检查是否可以工作在八线模式下
//     #define SPICOMMON_BUSFLAG_NATIVE_PINS   SPICOMMON_BUSFLAG_IOMUX_PINS
//     #define SPICOMMON_BUSFLAG_SLP_ALLOW_PD  (1<<9)     ///< Allow to power down the peripheral during light sleep, and auto recover then.

//      */
//     esp_err_t err = spi_bus_initialize(SPI2_HOST, &busfg, SPI_DMA_DISABLED);
//     if (err != ESP_OK)
//     {
//         ESP_LOGE(TAG, "spi_init spi_bus_initialize err: %d", err);
//         return;
//     }
//     spi_device_interface_config_t devcfg = {
//         .clock_speed_hz = SPI_HOST_FREQ, // 4M HZ
//         .mode = 0,                       // 模式0
//         .spics_io_num = PIN_NUM_CS,
//         .queue_size = 4, // 传输队列大小，使用spi_device_queue_trans接口可以将传输请求进行排队处理
//     };
//     /*
//     #define SPI_DEVICE_TXBIT_LSBFIRST          (1<<0)  /// 传输-低位先行
//     #define SPI_DEVICE_RXBIT_LSBFIRST          (1<<1)  /// 接收-低位先行
//     #define SPI_DEVICE_BIT_LSBFIRST            (SPI_DEVICE_TXBIT_LSBFIRST|SPI_DEVICE_RXBIT_LSBFIRST) /// 收发都是低位先行
//     #define SPI_DEVICE_3WIRE                   (1<<2)  ///使用三线模式，MOSI即为输入又为输出
//     #define SPI_DEVICE_POSITIVE_CS             (1<<3)  /// cs拉高选中
//     #define SPI_DEVICE_HALFDUPLEX              (1<<4)  /// 设置为半双工模式
//     #define SPI_DEVICE_CLK_AS_CS               (1<<5)  /// 把时钟线作为片选线
//     #define SPI_DEVICE_NO_DUMMY                (1<<6)
//     #define SPI_DEVICE_DDRCLK                  (1<<7)
//     #define SPI_DEVICE_NO_RETURN_RESULT        (1<<8)
//      */
//     err = spi_bus_add_device(SPI2_HOST, &devcfg, &spi);
//     if (err != ESP_OK)
//     {
//         ESP_LOGE(TAG, "spi_init spi_bus_add_device err: %d", err);
//         return;
//     }
//     return;
// }

// void app_main(void)
// {
//     while (1)
//     {
//         test();
//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }
// }