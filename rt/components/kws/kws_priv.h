#pragma once

#include "kws.h"

#ifdef __cplusplus
extern "C" {
#endif
void            guess_init(size_t inputs_sz);
int             guess_16b_16k_mono(float* feat);
#ifdef __cplusplus
}
#endif