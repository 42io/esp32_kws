#include "kws_priv.h"
#include <freertos/FreeRTOS.h>
#include "esp_log.h"
#include "non-copyable.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
#include <list>

/*****************************************************************************/

static const char* TAG = "guess";

static constexpr size_t kTensorArenaSize = 18048;

static constexpr size_t GUESS_INPUT_SZ = 13 * sizeof(float);

extern const uint8_t binary_model_start[] asm("_binary_dcnn_tflite_start");

static_assert(sizeof(float) == 4, "WTF");

/*****************************************************************************/

static void* aligned_malloc(size_t align, size_t size)
{
  size_t space = size + align - 1;
  auto ptr = malloc(space);
  if(ptr)
  {
    // The behavior is undefined if alignment is not a power of two.
    // The function modifies the pointer only if it would be possible to fit.
    configASSERT(!(align & (align - 1)));
    if(!std::align(align, size, ptr, space))
    {
      free(ptr);
      ptr = nullptr;
    }
  }
  return ptr;
}

/*****************************************************************************/

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

struct guess_t : NonCopyable
{
  tflite::MicroErrorReporter error_reporter;
  tflite::AllOpsResolver resolver;
  tflite::MicroInterpreter interpreter;
  TfLiteTensor* input = nullptr;
  TfLiteTensor* output = nullptr;
  std::list<StreamingState> streaming_state;
  alignas(16) uint8_t tensor_arena[kTensorArenaSize];
  guess_t(const tflite::Model* model);
  int guess(float* feat);
};

/*****************************************************************************/

guess_t::guess_t(const tflite::Model* model):
                 interpreter(model, resolver, tensor_arena, kTensorArenaSize, &error_reporter)
{
  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter.AllocateTensors();
  configASSERT(allocate_status == kTfLiteOk);
  ESP_LOGW(TAG, "arena used bytes %d", interpreter.arena_used_bytes());
  configASSERT(kTensorArenaSize == interpreter.arena_used_bytes());

  // Obtain pointers to the model's input and output tensors.
  input  = interpreter.input(0);
  output = interpreter.output(0);
  configASSERT(input->type == kTfLiteFloat32);
  configASSERT(input->bytes == GUESS_INPUT_SZ);
  configASSERT(output->type == kTfLiteFloat32);
  configASSERT(output->bytes == 12 * sizeof(float));

  configASSERT(interpreter.inputs_size() == interpreter.outputs_size());
  configASSERT(interpreter.inputs_size() > 2);

  // first conv layer dilation == 1
  streaming_state.emplace_back(interpreter.input(1), interpreter.output(1));

  // conv dilation > 1 layers
  size_t last = interpreter.inputs_size() - 1;
  for(size_t i = 2; i < last; i++)
    streaming_state.emplace_back(interpreter.input(i), interpreter.output(i), i);

  // last pooling layer
  streaming_state.emplace_back(interpreter.input(last), interpreter.output(last));

  // quick benchmark
  int64_t t1 = esp_timer_get_time();
  configASSERT(interpreter.Invoke() == kTfLiteOk);
  ESP_LOGW(TAG,"invoke time %f", ((float)(esp_timer_get_time() - t1))/1000000);
}

/*****************************************************************************/

int guess_t::guess(float* feat)
{
  // Fill input
  memcpy(input->data.f, feat, input->bytes);

  // Does nothing for non-streaming model
  for(auto& item : streaming_state)
    item.stream();

  // Run inference
  configASSERT(interpreter.Invoke() == kTfLiteOk);
  // for(size_t i = 0; i < output->bytes / sizeof(float); i++)
  // {
  //   printf("%f%c", output->data.f[i], i == output->bytes / sizeof(float) - 1 ? '\n' : ' ');
  // }
  int idx = -1;
  float max;
  for(size_t i = 0; i < output->bytes / sizeof(float); i++)
  {
    if(i == 0 || output->data.f[i] > max)
    {
      max = output->data.f[i];
      idx = i;
    }
  }
  configASSERT(idx >= 0);
  return idx;
}

/*****************************************************************************/

guess_t *guess_create(size_t inputs_sz)
{
  configASSERT(GUESS_INPUT_SZ == inputs_sz);

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  auto model = tflite::GetModel(binary_model_start);
  configASSERT(model->version() == TFLITE_SCHEMA_VERSION);

  void* buf = aligned_malloc(alignof(guess_t), sizeof(guess_t));
  configASSERT(buf);

  return new(buf) guess_t(model);
}

/*****************************************************************************/

int guess_16b_16k_mono(guess_t *instance, float* feat)
{
  return instance->guess(feat);
}

/*****************************************************************************/
