#pragma once

#include <stdint.h>
#include <stddef.h>

void* kws_init(size_t rate, size_t channels,
               size_t sample_bits, size_t buf_sz,
               void (*callback)(int word));

void  kws_detect(void);