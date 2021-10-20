#include "tf-streaming.h"
#include <freertos/FreeRTOS.h>
#include <string.h>

/*****************************************************************************/

void StreamingState::init(char* ii, size_t is, char* oo, size_t os)
{
  o = oo, i = ii, i_sz = is, o_sz = os;
  configASSERT(i_sz > o_sz);
  memset(i, 0, i_sz);
  r = new char[i_sz]();
  configASSERT(r != nullptr);
  mv_sz = i_sz - o_sz;
}

/*****************************************************************************/

StreamingState::StreamingState(TfLiteTensor* ii, TfLiteTensor* oo)
{
  configASSERT(oo->type == kTfLiteFloat32);
  configASSERT(ii->type == kTfLiteFloat32);
  configASSERT(ii->bytes % oo->bytes == 0);
  init(ii->data.raw, ii->bytes, oo->data.raw, oo->bytes);
}

/*****************************************************************************/

StreamingState::StreamingState(TfLiteTensor* ii, TfLiteTensor* oo, size_t d)
{
  configASSERT(oo->type == kTfLiteFloat32);
  configASSERT(ii->type == kTfLiteFloat32);
  configASSERT(ii->bytes % oo->bytes == 0);
  configASSERT(ii->bytes / oo->bytes == 2);
  configASSERT(d > 1);
  size_t x = 1 + (1 << (d - 1));
  init(ii->data.raw, x * oo->bytes, oo->data.raw, oo->bytes);
  strategy = &dilated_strategy_impl;
}

/*****************************************************************************/

void StreamingState::default_strategy_impl(StreamingState* self)
{
  memmove(self->r, &self->r[self->o_sz], self->mv_sz);
  memcpy(&self->r[self->mv_sz], self->o, self->o_sz);
  memcpy(self->i, self->r, self->i_sz);
}

/*****************************************************************************/

void StreamingState::dilated_strategy_impl(StreamingState* self)
{
  memmove(self->r, &self->r[self->o_sz], self->mv_sz);
  memcpy(&self->r[self->mv_sz], self->o, self->o_sz);
  memcpy(self->i, self->r, self->o_sz);
  memcpy(&self->i[self->o_sz], &self->r[self->mv_sz], self->o_sz);
}

/*****************************************************************************/
