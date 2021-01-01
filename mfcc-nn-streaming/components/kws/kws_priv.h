#pragma once

#include "kws.h"

#ifdef __cplusplus
extern "C" {
#endif
struct  guess_t;
struct  guess_t *guess_create(size_t inputs_sz);
int     guess_16b_16k_mono(struct guess_t *instance, float* feat);
#ifdef __cplusplus
}
#endif