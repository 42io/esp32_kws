#include "kws_priv.h"
#include <freertos/FreeRTOS.h>
#include "esp_log.h"
#include "non-copyable.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

/*****************************************************************************/

static_assert(sizeof(float) == 4, "WTF");

static const char* TAG = "guess";

static constexpr int kTensorArenaSize = 20648;

extern const uint8_t binary_model_start[] asm("_binary_dcnn_quant_tflite_start");

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

struct guess_t : NonCopyable
{
  tflite::MicroErrorReporter error_reporter;
  tflite::AllOpsResolver resolver;
  tflite::MicroInterpreter interpreter;
  TfLiteTensor* input = nullptr;
  TfLiteTensor* output = nullptr;
  alignas(16) uint8_t tensor_arena[kTensorArenaSize];
  guess_t(const tflite::Model* model, size_t inputs_sz);
  int guess(float* feat);
};

/*****************************************************************************/

guess_t::guess_t(const tflite::Model* model, size_t inputs_sz):
                 interpreter(model, resolver, tensor_arena, kTensorArenaSize, &error_reporter)
{
  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter.AllocateTensors();
  configASSERT(allocate_status == kTfLiteOk);
  ESP_LOGW(TAG, "arena used bytes %d\n", interpreter.arena_used_bytes());
  configASSERT(kTensorArenaSize == interpreter.arena_used_bytes());

  // Obtain pointers to the model's input and output tensors.
  input = interpreter.input(0);
  output = interpreter.output(0);

  configASSERT(input->type == kTfLiteFloat32);
  configASSERT(input->bytes == inputs_sz);

  configASSERT(output->type == kTfLiteFloat32);
  configASSERT(output->bytes == 12 * sizeof(float));

  int64_t t1 = esp_timer_get_time();
  configASSERT(interpreter.Invoke() == kTfLiteOk);
  ESP_LOGW(TAG,"invoke time %f", ((float)(esp_timer_get_time() - t1))/1000000);
}

/*****************************************************************************/

int guess_t::guess(float* feat)
{
  memcpy(input->data.f, feat, input->bytes);
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
  configASSERT(47 * 13 * sizeof(float) == inputs_sz);

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  auto model = tflite::GetModel(binary_model_start);
  configASSERT(model->version() == TFLITE_SCHEMA_VERSION);

  void* buf = aligned_malloc(alignof(guess_t), sizeof(guess_t));
  configASSERT(buf);

  return new(buf) guess_t(model, inputs_sz);
}

/*****************************************************************************/

int guess_16b_16k_mono(guess_t *instance, float* feat)
{
  return instance->guess(feat);
}

/*****************************************************************************/
