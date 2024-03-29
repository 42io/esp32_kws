#pragma once

#include "kws.h"

#ifdef __cplusplus
extern "C" {
#endif

struct  guess_t;
struct  guess_t *guess_create(size_t inputs_sz);
int     guess_16b_16k_mono(struct guess_t *instance, float* feat);

struct  confirm_t;
struct  confirm_t *confirm_create(size_t inputs_sz);
float   confirm_16b_16k_mono(struct confirm_t *instance, float* feat);

#ifdef __cplusplus
}
#endif