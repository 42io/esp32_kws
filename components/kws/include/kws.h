#pragma once

#include <stdint.h>

void kws_init(const char* name);

void kws_deinit();

const float* kws_guess_one_sec_16b_16k_mono(int16_t samples[16000]);