#include "kws_priv.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "tensorflow/lite/micro/kernels/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
#include <freertos/FreeRTOS.h>

extern "C" {
  #include "fe.h"
}

#if   CONFIG_KWS_MODEL_TYPE_DCNN
extern const uint8_t binary_model_start[] asm("_binary_dcnn_tflite_start");
extern const uint8_t binary_model_end[]   asm("_binary_dcnn_tflite_end");
#elif CONFIG_KWS_MODEL_TYPE_CNN
extern const uint8_t binary_model_start[] asm("_binary_cnn_tflite_start");
extern const uint8_t binary_model_end[]   asm("_binary_cnn_tflite_end");
#elif CONFIG_KWS_MODEL_TYPE_MLP
extern const uint8_t binary_model_start[] asm("_binary_mlp_tflite_start");
extern const uint8_t binary_model_end[]   asm("_binary_mlp_tflite_end");
#endif

static const char* TAG = "kws";

static_assert(sizeof(csf_float) == 4, "WTF");
static_assert(sizeof(float) == sizeof(csf_float), "WTF");

/*****************************************************************************/

// Globals, used for compatibility with Arduino-style sketches.
namespace {
  tflite::ErrorReporter* error_reporter = nullptr;
  const tflite::Model* model = nullptr;
  tflite::MicroInterpreter* interpreter = nullptr;
  TfLiteTensor* input = nullptr;
  TfLiteTensor* output = nullptr;
  const int kTensorArenaSize = binary_model_end - binary_model_start;
}  // namespace

/*****************************************************************************/

void kws_init()
{
    ESP_LOGW(TAG, "backend: TensorFlow Lite for Microcontrollers");

    uint8_t *tensor_arena = (uint8_t *)heap_caps_calloc(1, kTensorArenaSize, MALLOC_CAP_SPIRAM);
    configASSERT(tensor_arena);

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
    static tflite::ops::micro::AllOpsResolver resolver;

    // Build an interpreter to run the model with.
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    // Allocate memory from the tensor_arena for the model's tensors.
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    configASSERT(allocate_status == kTfLiteOk);

    // Obtain pointers to the model's input and output tensors.
    input = interpreter->input(0);
    output = interpreter->output(0);

    configASSERT(input->type == kTfLiteFloat32);
    configASSERT(input->bytes == 637 * sizeof(float));

    configASSERT(output->type == kTfLiteFloat32);
    configASSERT(output->bytes == 12 * sizeof(float));

    kws_detect_init();
}

/*****************************************************************************/

static float* kws_fe_one_sec_16b_16k_mono(audio_sample_t samples[16000])
{
    int n_frames, n_items_in_frame;
    csf_float *feat;

    n_frames = n_items_in_frame = 0;
    feat = fe_mfcc_16k_16b_mono(samples, 16000, &n_frames, &n_items_in_frame);
    configASSERT(n_frames == 49);
    configASSERT(n_items_in_frame == 13);
    configASSERT(n_items_in_frame > 0);
    configASSERT(feat);

    // for(int i = 0, idx = 0; i < n_frames; i++)
    // {
    //     for(int k = 0; k < n_items_in_frame; k++, idx++)
    //     {
    //         if(k)
    //         {
    //             printf(" ");
    //         }
    //         printf("%.3f", feat[idx]);
    //     }
    //     printf("\n");
    // }

    return feat;
}

/*****************************************************************************/

static void kws_guess_fe(float* feat)
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

int kws_guess_one_sec_16b_16k_mono(audio_sample_t* samples)
{
  float* feat = kws_fe_one_sec_16b_16k_mono(samples);
  kws_guess_fe(feat);
  int idx = vec_to_idx();
  free(feat);
  return idx;
}

/*****************************************************************************/