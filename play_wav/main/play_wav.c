#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_random.h"
#include "tf.h"
#include "format_wav.h"
#include "play.h"
#include "record.h"
#include "esp_log.h"

char *generate_random_string(size_t length)
{
    char *charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    if (length == 0 || charset == NULL || *charset == '\0')
    {
        return NULL;
    }

    const size_t charset_len = strlen(charset);
    char *result = malloc(length + 1); // +1 用于字符串结束符

    if (result == NULL)
    {
        return NULL;
    }

    for (size_t i = 0; i < length; i++)
    {
        // 使用esp_random()生成硬件随机数
        uint32_t random_value = esp_random();
        // 将随机数映射到字符集索引
        size_t index = random_value % charset_len;
        result[i] = charset[index];
    }

    result[length] = '\0'; // 添加字符串结束符
    return result;
}

void app_main(void)
{
    init_tf();
    // list_dir(MOUNT_POINT "/");
    // wav_header_t *header = (wav_header_t *)malloc(sizeof(wav_header_t));
    // read_wav_header(MOUNT_POINT "/16k16bit.wav", header);
    // print_wav_header(header);

    // i2s_play(MOUNT_POINT "/16k16bit.wav");
    i2s_play("/sdcard/CbMe6.wav");

    // char filename[18];
    // sprintf(filename, MOUNT_POINT "/%s.wav", generate_random_string(5));
    // ESP_LOGI("main", "filename: %s", filename);
    // i2s_record(filename, 10);

    unmount_tf();
}
