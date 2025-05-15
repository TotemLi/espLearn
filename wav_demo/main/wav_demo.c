#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_std.h"
#include "driver/gpio.h"

// INMP引脚，根据自己连线修改
#define INMP_SD GPIO_NUM_13
#define INMP_SCK GPIO_NUM_14
#define INMP_WS GPIO_NUM_12

// MAX98357A引脚，根据自己连线修改
#define MAX_DIN GPIO_NUM_15
#define MAX_BCLK GPIO_NUM_16
#define MAX_LRC GPIO_NUM_17

// 配置rx对INMP441的采样率为44.1kHz，这是常用的人声采样率
#define SAMPLE_RATE 44100

// buf size计算方法：根据esp32官方文档，buf size = dma frame num * 声道数 * 数据位宽 / 8
#define BUF_SIZE (1023 * 1 * 32 / 8)

// 音频buffer
uint8_t buf[BUF_SIZE];

i2s_chan_handle_t rx_handle;
i2s_chan_handle_t tx_handle;

// 初始化i2s rx，用于从INMP441接收数据
static void i2s_rx_init(void)
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);

    // dma frame num使用最大值，增大dma一次搬运的数据量，能够提高效率，减小杂音，使用1023可以做到没有一丝杂音
    chan_cfg.dma_frame_num = 1023;
    i2s_new_channel(&chan_cfg, NULL, &rx_handle);

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),

        // 虽然inmp441采集数据为24bit，但是仍可使用32bit来接收，中间存储过程不需考虑，只要让声音怎么进来就怎么出去即可
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_24BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .dout = I2S_GPIO_UNUSED,
            .bclk = INMP_SCK,
            .ws = INMP_WS,
            .din = INMP_SD,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };

    i2s_channel_init_std_mode(rx_handle, &std_cfg);

    i2s_channel_enable(rx_handle);
}

// 初始化tx，用于向MAX98357A写数据
static void i2s_tx_init(void)
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_1, I2S_ROLE_MASTER);
    chan_cfg.dma_frame_num = 1023;
    i2s_new_channel(&chan_cfg, &tx_handle, NULL);

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_24BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .din = I2S_GPIO_UNUSED,
            .bclk = MAX_BCLK,
            .ws = MAX_LRC,
            .dout = MAX_DIN,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };

    i2s_channel_init_std_mode(tx_handle, &std_cfg);

    i2s_channel_enable(tx_handle);
}

static void i2s_read_task(void *args)
{
    size_t bytes = 0;

    // 一次性读取buf_size数量的音频，即dma最大搬运一次的数据量，读成功后，写入tx，即可通过MAX98357A播放
    while (1)
    {
        if (i2s_channel_read(rx_handle, buf, BUF_SIZE, &bytes, 1000) == ESP_OK)
        {
            i2s_channel_write(tx_handle, buf, BUF_SIZE, &bytes, 1000);
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    i2s_tx_init();

    i2s_rx_init();

    xTaskCreate(i2s_read_task, "i2s_read_task", 4096 * 2, NULL, tskIDLE_PRIORITY, NULL);
}