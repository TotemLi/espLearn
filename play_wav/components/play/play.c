#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "play.h"
#include "format_wav.h"
#include "esp_log.h"

static const char *TAG = "play_wav";

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
