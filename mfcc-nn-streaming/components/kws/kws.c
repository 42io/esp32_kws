#include <assert.h>
#include <string.h>
#include <sys/queue.h>
#include "kws_priv.h"
#include "fe.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

/*****************************************************************************/

_Static_assert(sizeof(csf_float) == 4, "WTF");
_Static_assert(sizeof(float) == sizeof(csf_float), "WTF");

/*****************************************************************************/

typedef int16_t audio_sample_t;

/*****************************************************************************/

#define KWS_SAMPLE_RATE_HZ   (16000)
#define KWS_RAW_CHUNK_SZ     (1600 * sizeof(audio_sample_t))
#define KWS_RAW_RING_SZ      (2 * KWS_RAW_CHUNK_SZ)
#define KWS_MFCC_FRAME_LEN   (13)

/*****************************************************************************/

static char                  ring[KWS_RAW_RING_SZ];
static xQueueHandle          queue;
struct guess_t              *guess;
static void (*on_detected)(int word);

/*****************************************************************************/

static csf_float* kws_fe_16b_16k_mono(audio_sample_t samples[3200])
{
  int n_frames, n_items_in_frame;
  csf_float *feat;

  n_frames = n_items_in_frame = 0;
  feat = fe_mfcc_16k_16b_mono(samples, 2400, &n_frames, &n_items_in_frame);
  assert(n_frames == 6);
  assert(n_items_in_frame == KWS_MFCC_FRAME_LEN);
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
  void *buf = malloc(KWS_RAW_RING_SZ);
  assert(buf);

  for(;;)
  {
    xEventGroupWaitAllBitsAndClear(event, BIT0);
    xQueueReceive(queue, buf, portMAX_DELAY);
    xEventGroupSetBits(event, BIT1);

    csf_float (*feat)[KWS_MFCC_FRAME_LEN] = (csf_float(*)[]) kws_fe_16b_16k_mono(buf);

    xEventGroupWaitAllBitsAndClear(event, BIT0);
    for(int i = 1; i < 6; i++) {
      int word = guess_16b_16k_mono(guess, feat[i]);
      on_detected(word);
    }
    xEventGroupSetBits(event, BIT1);

    free(feat);
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

  assert(on_detected = callback);
  assert(queue = xQueueCreate(2, KWS_RAW_RING_SZ));
  assert(guess = guess_create(sizeof(csf_float) * KWS_MFCC_FRAME_LEN));
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
