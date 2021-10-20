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
#include "tf-streaming.h"

/*****************************************************************************/

static const char* TAG = "confirm";

static constexpr size_t kTensorArenaSize = CONFIG_KWS_CONFIRM_TENSOR_ARENA_SZ;

static constexpr size_t GUESS_INPUT_SZ = 13 * sizeof(float);

extern const uint8_t guess_model_start[] asm(CONFIG_KWS_CONFIRM_MODEL_START);

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

struct confirm_t : NonCopyable
{
  tflite::MicroErrorReporter error_reporter;
  tflite::AllOpsResolver resolver;
  tflite::MicroInterpreter interpreter;
  TfLiteTensor* input = nullptr;
  TfLiteTensor* output = nullptr;
  std::list<StreamingState> streaming_state;
  alignas(16) uint8_t tensor_arena[kTensorArenaSize];
  confirm_t(const tflite::Model* model);
  float confirm(float* feat);
};

/*****************************************************************************/

confirm_t::confirm_t(const tflite::Model* model):
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
  configASSERT(output->bytes == 1 * sizeof(float));

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
  for(size_t i = 0; i < 47; i++)
  {
    memset(input->data.f, 0, input->bytes);
    for(auto& item : streaming_state)
      item.stream();
    configASSERT(interpreter.Invoke() == kTfLiteOk);
  }
  ESP_LOGW(TAG,"invoke time %f", ((float)(esp_timer_get_time() - t1))/1000000/47);

  const float t2 = -6.06364584;

  configASSERT(output->bytes == sizeof t2);
  configASSERT(output->data.f[0] == t2);

  // printf("%.9g\n", output->data.f[0]);
}

/*****************************************************************************/

float confirm_t::confirm(float* feat)
{
  // Fill input
  memcpy(input->data.f, feat, input->bytes);

  // Does nothing for non-streaming model
  for(auto& item : streaming_state)
    item.stream();

  // Run inference
  configASSERT(interpreter.Invoke() == kTfLiteOk);

  return output->data.f[0];
}

/*****************************************************************************/

confirm_t *confirm_create(size_t inputs_sz)
{
  configASSERT(GUESS_INPUT_SZ == inputs_sz);

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  auto model = tflite::GetModel(guess_model_start);
  configASSERT(model->version() == TFLITE_SCHEMA_VERSION);

  void* buf = aligned_malloc(alignof(confirm_t), sizeof(confirm_t));
  configASSERT(buf);

  return new(buf) confirm_t(model);
}

/*****************************************************************************/

float confirm_16b_16k_mono(confirm_t *instance, float* feat)
{
  return instance->confirm(feat);
}

/*****************************************************************************/
