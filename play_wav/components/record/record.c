#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "record.h"
#include "freertos/FreeRTOS.h"
#include "play.h"
#include "format_wav.h"
#include "esp_log.h"
#include "driver/i2s_std.h"
#include "driver/gpio.h"

static const char *TAG = "record_wav";
static i2s_chan_handle_t rx_chan;

#define BCLK_IO GPIO_NUM_13
#define WS_IO GPIO_NUM_14
#define DOUT_IO GPIO_NUM_15

#define SAMPLE_RATE 44100

#define BUF_SIZE (1023 * 1 * 32 / 8)
uint8_t buf[BUF_SIZE];

void i2s_init_record()
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_1, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &rx_chan, NULL));

    i2s_std_config_t tx_std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_24BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = BCLK_IO, // 位时钟
            .ws = WS_IO,     // 左右声道选择
            .dout = DOUT_IO, // 输出
            .din = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_chan, &tx_std_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(rx_chan));
}

void i2s_record(const char *path)
{
    esp_err_t err;
    FILE *f = fopen(path, "w");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "can't open file: %s, errno: %d", path, errno);
        return;
    }

    wav_header_t header = WAV_HEADER_PCM_DEFAULT(0, I2S_DATA_BIT_WIDTH_24BIT, SAMPLE_RATE, I2S_SLOT_MODE_MONO);
    size_t written_len = fwrite(&header, 1, sizeof(wav_header_t), f);
    if (written_len != sizeof(wav_header_t))
    {
        ESP_LOGE(TAG, "write err, write len: %d", written_len);
        fclose(f);
        return;
    }

    size_t wav_sample_size = 0;
    size_t read_num = 0;
    while (1)
    {
        err = i2s_channel_read(rx_chan, buf, BUF_SIZE, &read_num, portMAX_DELAY);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "i2s_channel_read err: %s", esp_err_to_name(err));
            break;
        }
        size_t written_len = fwrite(buf, 1, read_num, f);
        if (written_len != read_num)
        {
            ESP_LOGE(TAG, "write err, write len: %d", written_len);
            break;
        }
        wav_sample_size++;
    }
    // TODO 更新wav header 中size的信息

    fclose(f);
}
