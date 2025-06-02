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

#define BCLK_IO GPIO_NUM_11 // SCK
#define WS_IO GPIO_NUM_12   // WS
#define DOUT_IO GPIO_NUM_13 // SD

#define SAMPLE_RATE 44100
#define SAMPLE_BITS I2S_DATA_BIT_WIDTH_24BIT
#define BYTE_RATE (SAMPLE_RATE * (SAMPLE_BITS / 8)) * 1
#define SAMPLE_SIZE (SAMPLE_BITS * 1024)

static char i2s_readraw_buff[SAMPLE_SIZE];

void i2s_init_record()
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_1, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, NULL, &rx_chan));

    i2s_std_config_t tx_std_cfg = {
        // .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
        .clk_cfg = {
            .sample_rate_hz = SAMPLE_RATE,
            .clk_src = I2S_CLK_SRC_DEFAULT,
            .ext_clk_freq_hz = 0,
            .mclk_multiple = I2S_MCLK_MULTIPLE_384,
            .bclk_div = 8,
        },
        // .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(SAMPLE_BITS, I2S_SLOT_MODE_MONO),
        .slot_cfg = {
            .data_bit_width = SAMPLE_BITS,
            .slot_bit_width = I2S_SLOT_BIT_WIDTH_AUTO,
            .slot_mode = I2S_SLOT_MODE_MONO,
            .slot_mask = I2S_STD_SLOT_LEFT,
            .ws_width = SAMPLE_BITS,
            .ws_pol = false,
            .bit_shift = false,
            .left_align = true,
            .big_endian = false,
            .bit_order_lsb = false,
        },
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = BCLK_IO,         // 位时钟
            .ws = WS_IO,             // 左右声道选择
            .dout = I2S_GPIO_UNUSED, // 输出
            .din = DOUT_IO,
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

void i2s_record(const char *path, uint32_t rec_time)
{
    esp_err_t err;
    int flash_wr_size = 0;
    i2s_init_record();

    // size_t read_num = 0;
    // ESP_LOGI(TAG, "print buf content");
    // while (1)
    // {
    //     err = i2s_channel_read(rx_chan, buf, BUF_SIZE, &read_num, portMAX_DELAY);
    //     if (err != ESP_OK)
    //     {
    //         ESP_LOGE(TAG, "i2s_channel_read err: %s", esp_err_to_name(err));
    //         break;
    //     }
    //     ESP_LOG_BUFFER_HEXDUMP("TAG", buf, BUF_SIZE, ESP_LOG_INFO);
    //     vTaskDelay(pdMS_TO_TICKS(1000));
    // }

    FILE *f = fopen(path, "w");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "can't open file: %s, errno: %d", path, errno);
        return;
    }

    uint32_t flash_rec_time = BYTE_RATE * rec_time;
    wav_header_t header = WAV_HEADER_PCM_DEFAULT(flash_rec_time, SAMPLE_BITS, SAMPLE_RATE, I2S_SLOT_MODE_MONO);
    fwrite(&header, sizeof(header), 1, f);
    // size_t written_len = fwrite(&header, sizeof(header), 1, f);
    // if (written_len != sizeof(header))
    // {
    //     ESP_LOGE(TAG, "write err, write len: %d", written_len);
    //     fclose(f);
    //     return;
    // }

    size_t read_num = 0;
    ESP_LOGI(TAG, "Recording begin!");
    while (flash_wr_size < flash_rec_time)
    {
        err = i2s_channel_read(rx_chan, i2s_readraw_buff, SAMPLE_SIZE, &read_num, portMAX_DELAY);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "i2s_channel_read err: %s", esp_err_to_name(err));
            break;
        }
        fwrite(i2s_readraw_buff, read_num, 1, f);
        // size_t written_len = fwrite(i2s_readraw_buff, read_num, 1, f);
        // if (written_len != read_num)
        // {
        //     ESP_LOGE(TAG, "write err, write len: %d", written_len);
        //     break;
        // }
        flash_wr_size += read_num;
    }
    ESP_LOGI(TAG, "Recording done!");
    fclose(f);
}
