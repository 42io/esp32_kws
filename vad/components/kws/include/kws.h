#pragma once

#include <stdint.h>

typedef int16_t audio_sample_t;

void  kws_init(const char* name);
int   kws_get_samp_rate();
int   kws_get_samp_chunksize();
int   kws_detect(audio_sample_t* chunk);

audio_sample_t* kws_get_next_chunk();