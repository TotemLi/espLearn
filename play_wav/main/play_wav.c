#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "tf.h"
#include "format_wav.h"
#include "play.h"

void app_main(void)
{
    init_tf();

    wav_header_t *header = (wav_header_t *)malloc(sizeof(wav_header_t));
    read_wav_header(MOUNT_POINT "test.wav", header);
}
