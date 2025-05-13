#pragma once

#include "format_wav.h"

void read_wav_header(const char *path, wav_header_t *header);
void print_wav_header(wav_header_t *header);
