#include <assert.h>
#include <string.h>
#include <sys/queue.h>
#include "kws_priv.h"
#include "fe.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_log.h"

/*****************************************************************************/

_Static_assert(sizeof(float) == 4, "WTF");

/*****************************************************************************/

typedef int16_t audio_sample_t;

/*****************************************************************************/

#define KWS_SAMPLE_RATE_HZ   (16000)
#define KWS_RAW_CHUNK_SZ     (1600 * sizeof(audio_sample_t))
#define KWS_QUEUE_ITEM_SZ    (2400 * sizeof(audio_sample_t))
#define KWS_MFCC_CHUNK_SZ    (5 * 13 * sizeof(float))
#define KWS_MFCC_RING_SZ     (47 * 13 * sizeof(float))

/*****************************************************************************/

static char                  ring[2 * KWS_RAW_CHUNK_SZ];
static char                  mfcc[KWS_MFCC_RING_SZ];
static xQueueHandle          queue;
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
  assert(n_frames == 6);
  assert(n_items_in_frame == 13);
  assert(feat);

  return feat;
}

/*****************************************************************************/

static void xEventGroupWaitAllBitsAndClear(EventGroupHandle_t e, EventBits_t b)
{
  xEventGroupWaitBits(e, b, pdTRUE /* clear */, pdTRUE /* all */, portMAX_DELAY);
}

/*****************************************************************************/

static void fe_task(void *parameters)
{
  const EventGroupHandle_t event = parameters;
  char *buf = malloc(KWS_QUEUE_ITEM_SZ);
  assert(buf);
  struct guess_t *guess = guess_create(KWS_MFCC_RING_SZ);
  assert(guess);

  _Static_assert(KWS_QUEUE_ITEM_SZ >= KWS_MFCC_RING_SZ, "WTF");

  for(;;)
  {
    xEventGroupWaitAllBitsAndClear(event, BIT0);
    xQueueReceive(queue, buf, portMAX_DELAY);
    xEventGroupSetBits(event, BIT1);

    float* feat = kws_fe_16b_16k_mono((audio_sample_t*)buf);

    xEventGroupWaitAllBitsAndClear(event, BIT0);
    memmove(mfcc, &mfcc[KWS_MFCC_CHUNK_SZ], KWS_MFCC_RING_SZ - KWS_MFCC_CHUNK_SZ);
    memcpy(&mfcc[KWS_MFCC_RING_SZ - KWS_MFCC_CHUNK_SZ], &feat[13], KWS_MFCC_CHUNK_SZ);
    memcpy(buf, mfcc, KWS_MFCC_RING_SZ);
    xEventGroupSetBits(event, BIT1);

    free(feat);
    const int word = guess_16b_16k_mono(guess, (float*)buf);

    xEventGroupWaitAllBitsAndClear(event, BIT0);
    on_detected(word);
    xEventGroupSetBits(event, BIT1);
  }
  vTaskDelete(NULL);
}

/*****************************************************************************/

static void kws_task(void *parameters)
{
  EventGroupHandle_t core0, core1;

  assert(core0 = xEventGroupCreate());
  assert(core1 = xEventGroupCreate());
  assert(xTaskCreatePinnedToCore(&fe_task, "worker_0", 3072, core0, 1, NULL, 0) == pdPASS);
  assert(xTaskCreatePinnedToCore(&fe_task, "worker_1", 3072, core1, 1, NULL, 1) == pdPASS);

  for(;;)
  {
    xEventGroupSetBits(core0, BIT0);
    xEventGroupWaitAllBitsAndClear(core0, BIT1);
    xEventGroupSetBits(core1, BIT0);
    xEventGroupWaitAllBitsAndClear(core1, BIT1);
  }
  vTaskDelete(NULL);
}

/*****************************************************************************/

void* kws_init(size_t rate, size_t channels, size_t sample_bits, size_t buf_sz,
               void (*callback)(int word))
{
  assert(buf_sz == KWS_RAW_CHUNK_SZ);
  assert(sample_bits == 8 * sizeof(audio_sample_t));
  assert(channels == 1);
  assert(rate == KWS_SAMPLE_RATE_HZ);

  fe_mfcc_init();
  int64_t t1 = esp_timer_get_time();
  free(kws_fe_16b_16k_mono((void*)ring));
  ESP_LOGW(TAG,"fe time %f", ((float)(esp_timer_get_time() - t1))/1000000);

  assert(on_detected = callback);
  assert(queue = xQueueCreate(3, KWS_QUEUE_ITEM_SZ));
  assert(xTaskCreate(&kws_task, "kws", 1024, NULL, 1, NULL) == pdPASS);

  return &ring[KWS_RAW_CHUNK_SZ];
}

/*****************************************************************************/

void kws_detect()
{
  assert(xQueueSend(queue, ring, 0) == pdPASS);
  memcpy(ring, &ring[KWS_RAW_CHUNK_SZ], KWS_RAW_CHUNK_SZ);
}

/*****************************************************************************/
