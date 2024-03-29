#include <assert.h>
#include <string.h>
#include "kws_priv.h"
#include "fe.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include <sys/param.h>  // For MIN/MAX(a, b)

/*****************************************************************************/

_Static_assert(sizeof(float) == 4, "WTF");

/*****************************************************************************/

typedef int16_t audio_sample_t;

/*****************************************************************************/

#define KWS_SAMPLE_RATE_HZ   (16000)
#define KWS_RAW_CHUNK_SZ     (1600 * sizeof(audio_sample_t))
#define KWS_QUEUE_ITEM_SZ    (2400 * sizeof(audio_sample_t))
#define KWS_MFCC_FRAME_LEN   (13)
#define KWS_MFCC_LEN         (6)

/*****************************************************************************/

static char                  ring[2 * KWS_RAW_CHUNK_SZ];
static xQueueHandle          queue;
static struct guess_t       *guess;
static struct confirm_t     *confirm;
static void (*on_detected)(int word);

static const char* TAG = "kws";

/*****************************************************************************/

static float* kws_fe_16b_16k_mono(audio_sample_t *samples)
{
  int n_frames, n_items_in_frame;
  float *feat;
  n_frames = n_items_in_frame = 0;

  _Static_assert(KWS_QUEUE_ITEM_SZ % sizeof(*samples) == 0, "WTF");

  feat = fe_mfcc_16k_16b_mono(
    samples, KWS_QUEUE_ITEM_SZ / sizeof(*samples),
    &n_frames, &n_items_in_frame);
  configASSERT(n_frames == KWS_MFCC_LEN);
  configASSERT(n_items_in_frame == KWS_MFCC_FRAME_LEN);
  configASSERT(feat);

  return feat;
}

/*****************************************************************************/

static void confirm_task(void *parameters)
{
  xQueueHandle* io = parameters;
  const xQueueHandle input  = io[0],
                     output = io[1];

  float feat[KWS_MFCC_FRAME_LEN];

  for(;;)
  {
    xQueueReceive(input, feat, portMAX_DELAY);
    float o = confirm_16b_16k_mono(confirm, feat);
    configASSERT(xQueueSend(output, &o, 0) == pdPASS);
  }
  vTaskDelete(NULL);
}

/*****************************************************************************/

static void kws_task(void *parameters)
{
  void *buf = malloc(KWS_QUEUE_ITEM_SZ);
  configASSERT(buf);

  xQueueHandle co_io[2];
  configASSERT(co_io[0] = xQueueCreate(1, KWS_MFCC_FRAME_LEN * sizeof(float)));
  configASSERT(co_io[1] = xQueueCreate(1, sizeof(float)));
  configASSERT(xTaskCreatePinnedToCore(&confirm_task, "confirm", 3072, co_io, 1, NULL,
                                       CONFIG_KWS_CONFIRM_PINNED_TO_CORE) == pdPASS);

  for(;;)
  {
    xQueueReceive(queue, buf, portMAX_DELAY);

    float (*feat)[KWS_MFCC_FRAME_LEN] = (float(*)[]) kws_fe_16b_16k_mono(buf);

    for(int i = 1; i < KWS_MFCC_LEN; i++) {
      configASSERT(xQueueSend(co_io[0], feat[i], 0) == pdPASS);
      int word = guess_16b_16k_mono(guess, feat[i]);
      float confidence;
      xQueueReceive(co_io[1], &confidence, portMAX_DELAY);
      on_detected(confidence > CONFIG_KWS_CONFIRM_THRESHOLD ? word : MAX(10, word));
    }

    free(feat);
  }
  vTaskDelete(NULL);
}

/*****************************************************************************/

static void kws_fe_init()
{
  fe_mfcc_init();
  int64_t t1 = esp_timer_get_time();
  float (*feat)[KWS_MFCC_FRAME_LEN] = (float(*)[]) kws_fe_16b_16k_mono((void*)ring);
  ESP_LOGW(TAG,"fe time %f", ((float)(esp_timer_get_time() - t1))/1000000);

  // unsigned char *y = (unsigned char*)feat;
  // for(int k = 0; k < sizeof(*feat); k++)
  // {
  //   printf("0x%02hx, ", y[k]);
  // }
  // printf("\n");

  const unsigned char dump[] = {
    0x50, 0xac, 0xae, 0xc2, 0x33, 0x9d, 0x88, 0xa9, 0x18, 0x85, 0x91, 0x29, 0x07,
    0x4b, 0x14, 0xaa, 0x39, 0xa0, 0x76, 0x2a, 0xc4, 0x04, 0x36, 0x2a, 0x72, 0xa4,
    0x4e, 0x2b, 0xda, 0x82, 0xe3, 0xaa, 0x25, 0x85, 0x12, 0x2b, 0xdc, 0xf5, 0xf9,
    0xab, 0x79, 0x04, 0xd3, 0xaa, 0x53, 0x51, 0x62, 0xac, 0xa2, 0x73, 0x2b, 0xab,
  };

  _Static_assert(sizeof(*feat) == sizeof dump, "WTF");

  for(int i = 0; i < KWS_MFCC_LEN; i++) {
    configASSERT(memcmp(dump, feat[i], sizeof dump) == 0);
  }

  free(feat);
}

/*****************************************************************************/

void* kws_init(size_t rate, size_t channels, size_t sample_bits, size_t buf_sz,
               void (*callback)(int word))
{
  configASSERT(buf_sz == KWS_RAW_CHUNK_SZ);
  configASSERT(sample_bits == 8 * sizeof(audio_sample_t));
  configASSERT(channels == 1);
  configASSERT(rate == KWS_SAMPLE_RATE_HZ);

  kws_fe_init();

  configASSERT(on_detected = callback);
  configASSERT(queue = xQueueCreate(3, KWS_QUEUE_ITEM_SZ));
  configASSERT(guess = guess_create(sizeof(float) * KWS_MFCC_FRAME_LEN));
  configASSERT(confirm = confirm_create(sizeof(float) * KWS_MFCC_FRAME_LEN));
  configASSERT(xTaskCreatePinnedToCore(&kws_task, "kws", 3072, NULL, 1, NULL,
                                       CONFIG_KWS_PINNED_TO_CORE) == pdPASS);

  return &ring[KWS_RAW_CHUNK_SZ];
}

/*****************************************************************************/

void kws_detect()
{
  configASSERT(xQueueSend(queue, ring, 0) == pdPASS);
  memcpy(ring, &ring[KWS_RAW_CHUNK_SZ], KWS_RAW_CHUNK_SZ);
}

/*****************************************************************************/
