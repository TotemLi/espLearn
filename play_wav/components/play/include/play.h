#pragma once

#include "format_wav.h"

void read_wav_header(const char *path, wav_header_t *header);
void print_wav_header(wav_header_t *header);
void i2s_init_play(uint32_t sample_rate, uint16_t bits_per_sample, uint16_t num_of_channels);
void i2s_play(const char *path);
