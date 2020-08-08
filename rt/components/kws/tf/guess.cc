#include "kws_priv.h"
#include "esp_log.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
#include <freertos/FreeRTOS.h>

_Static_assert(sizeof(float) == 4, "WTF");

extern const uint8_t binary_model_start[] asm("_binary_dcnn_quant_tflite_start");

static const char* TAG = "guess";

/*****************************************************************************/

// Globals, used for compatibility with Arduino-style sketches.
namespace {
  tflite::ErrorReporter* error_reporter = nullptr;
  const tflite::Model* model = nullptr;
  tflite::MicroInterpreter* interpreter = nullptr;
  TfLiteTensor* input = nullptr;
  TfLiteTensor* output = nullptr;
  constexpr int kTensorArenaSize = 12136;
  alignas(16) uint8_t tensor_arena[kTensorArenaSize];
}  // namespace

/*****************************************************************************/

void guess_init(size_t inputs_sz)
{
    ESP_LOGW(TAG, "TensorFlow Lite for Microcontrollers");

    configASSERT(47 * 13 * sizeof(float) == inputs_sz);

    // Set up logging. Google style is to avoid globals or statics because of
    // lifetime uncertainty, but since this has a trivial destructor it's okay.
    // NOLINTNEXTLINE(runtime-global-variables)
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    // Map the model into a usable data structure. This doesn't involve any
    // copying or parsing, it's a very lightweight operation.
    model = tflite::GetModel(binary_model_start);
    configASSERT(model->version() == TFLITE_SCHEMA_VERSION);

    // This pulls in all the operation implementations we need.
    // NOLINTNEXTLINE(runtime-global-variables)
    static tflite::AllOpsResolver resolver;

    // Build an interpreter to run the model with.
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    // Allocate memory from the tensor_arena for the model's tensors.
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    configASSERT(allocate_status == kTfLiteOk);

    ESP_LOGW(TAG,"arena used bytes %d", interpreter->arena_used_bytes());
    configASSERT(kTensorArenaSize == interpreter->arena_used_bytes());

    // Obtain pointers to the model's input and output tensors.
    input = interpreter->input(0);
    output = interpreter->output(0);

    configASSERT(input->type == kTfLiteFloat32);
    configASSERT(input->bytes == inputs_sz);

    configASSERT(output->type == kTfLiteFloat32);
    configASSERT(output->bytes == 12 * sizeof(float));

    int64_t t1 = esp_timer_get_time();
    configASSERT(interpreter->Invoke() == kTfLiteOk);
    ESP_LOGW(TAG,"invoke time %f", ((float)(esp_timer_get_time() - t1))/1000000);
}

/*****************************************************************************/

static void guess(float* feat)
{
    memcpy(input->data.f, feat, input->bytes);
    configASSERT(interpreter->Invoke() == kTfLiteOk);
    // for(size_t i = 0; i < output->bytes / sizeof(float); i++)
    // {
    //   printf("%f%c", output->data.f[i], i == output->bytes / sizeof(float) - 1 ? '\n' : ' ');
    // }
}

/*****************************************************************************/

static int vec_to_idx()
{
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

int guess_16b_16k_mono(float* feat)
{
  guess(feat);
  return vec_to_idx();
}

/*****************************************************************************/
