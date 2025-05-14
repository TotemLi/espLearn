#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "play.h"
#include "format_wav.h"
#include "esp_log.h"
#include "driver/i2s_std.h"
#include "driver/gpio.h"

static const char *TAG = "play_wav";
static i2s_chan_handle_t tx_chan;

#define BCLK_IO GPIO_NUM_1
#define WS_IO GPIO_NUM_3
#define DOUT_IO GPIO_NUM_2

void read_wav_header(const char *path, wav_header_t *header)
{
    FILE *fp = fopen(path, "rb");
    if (fp == NULL)
    {
        ESP_LOGI(TAG, "open file: %s, err: %s", path, strerror(errno));
        return;
    }

    size_t read_size = fread(header, 1, sizeof(wav_header_t), fp);
    if (read_size < sizeof(wav_header_t))
    {
        ESP_LOGI(TAG, "read header err: %s", strerror(errno));
        return;
    }
    fclose(fp);
    return;
}

void print_wav_header(wav_header_t *header)
{
    // 描述块信息
    ESP_LOGI(TAG, "描述块 (RIFF):\n");
    ESP_LOGI(TAG, "  标识: %.4s\n", header->descriptor_chunk.chunk_id);
    ESP_LOGI(TAG, "  大小: %" PRIu32 " 字节\n", header->descriptor_chunk.chunk_size);
    ESP_LOGI(TAG, "  格式: %.4s\n", header->descriptor_chunk.chunk_format);

    // 格式块信息
    ESP_LOGI(TAG, "格式块 (fmt):\n");
    ESP_LOGI(TAG, "  标识: %.4s\n", header->fmt_chunk.subchunk_id);
    ESP_LOGI(TAG, "  大小: %" PRIu32 " 字节\n", header->fmt_chunk.subchunk_size);
    ESP_LOGI(TAG, "  音频格式: %" PRIu16, header->fmt_chunk.audio_format);
    if (header->fmt_chunk.audio_format == 1)
        ESP_LOGI(TAG, "(PCM)\n");
    else
        ESP_LOGI(TAG, "(压缩格式)\n");
    ESP_LOGI(TAG, "  声道数: %" PRIu16, header->fmt_chunk.num_of_channels);
    if (header->fmt_chunk.num_of_channels == 1)
        ESP_LOGI(TAG, "(单声道)\n");
    else if (header->fmt_chunk.num_of_channels == 2)
        ESP_LOGI(TAG, "(立体声)\n");
    else
        ESP_LOGI(TAG, "(其他)\n");
    ESP_LOGI(TAG, "  采样率: %" PRIu32 " Hz\n", header->fmt_chunk.sample_rate);
    ESP_LOGI(TAG, "  字节率: %" PRIu32 " 字节/秒\n", header->fmt_chunk.byte_rate);
    ESP_LOGI(TAG, "  块对齐: %" PRIu16 " 字节\n", header->fmt_chunk.block_align);
    ESP_LOGI(TAG, "  位深度: %" PRIu16 " 位/样本\n", header->fmt_chunk.bits_per_sample);

    // 数据块信息
    ESP_LOGI(TAG, "数据块 (data):\n");
    ESP_LOGI(TAG, "  标识: %.4s\n", header->data_chunk.subchunk_id);
    ESP_LOGI(TAG, "  数据大小: %" PRIu32 " 字节\n", header->data_chunk.subchunk_size);
}

void i2s_init_play(uint32_t sample_rate, uint16_t bits_per_sample, uint16_t num_of_channels)
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_chan, NULL));

    i2s_std_config_t tx_std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(sample_rate),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(bits_per_sample, num_of_channels),
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
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_chan, &tx_std_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(tx_chan));
}

void i2s_play(const char *path)
{
    wav_header_t *header = (wav_header_t *)malloc(sizeof(wav_header_t));
    FILE *fp = fopen(path, "rb");
    if (fp == NULL)
    {
        ESP_LOGI(TAG, "open file: %s, err: %s", path, strerror(errno));
        return;
    }

    size_t read_size = fread(header, 1, sizeof(wav_header_t), fp);
    if (read_size < sizeof(wav_header_t))
    {
        ESP_LOGI(TAG, "read header err: %s", strerror(errno));
        return;
    }

    uint32_t buf_size;
    buf_size = header->fmt_chunk.num_of_channels * header->fmt_chunk.bits_per_sample * 16;
    uint8_t buf_data[buf_size];

    while (1)
    {
        read_size = fread(buf_data, 1, buf_size, fp);
        ESP_ERROR_CHECK(i2s_channel_write(tx_chan, buf_data, read_size, NULL, portMAX_DELAY));
        ESP_LOGI(TAG, "i2s_channel_write %d byte", read_size);

        if (read_size < buf_size)
        {
            ESP_LOGI(TAG, "i2s_channel_write done, err: %s", strerror(errno));
            break;
        }
    }

    fclose(fp);
}