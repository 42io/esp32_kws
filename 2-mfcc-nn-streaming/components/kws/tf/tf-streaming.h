#pragma once

#include "non-copyable.h"
#include "tensorflow/lite/c/common.h"

class StreamingState : NonCopyable
{
  char  *i, *o, *r;
  size_t i_sz, o_sz, mv_sz;
  void init(char* ii, size_t is, char* oo, size_t os);
  void (*strategy)(StreamingState* self) = &default_strategy_impl;
  static void default_strategy_impl(StreamingState* self);
  static void dilated_strategy_impl(StreamingState* self);

public:
  StreamingState(TfLiteTensor* ii, TfLiteTensor* oo);
  StreamingState(TfLiteTensor* ii, size_t ss);
  StreamingState(TfLiteTensor* ii, TfLiteTensor* oo, size_t d);
  ~StreamingState() { delete[] r; }
  void stream() { strategy(this); }
};