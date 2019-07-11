#include "kws_priv.h"
#include "esp_vad.h"
#include <assert.h>

#if CONFIG_KWS_DUMP_AUDIO_SAMPLE_TO_LOG
#include "mbedtls/base64.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

static vad_handle_t    vad_inst;
static audio_sample_t* word_buf;

static const int WORD_BUF_SZ = KWS_SAMPLES_IN_CHUNK
                               * KWS_CHUNKS_IN_WORD
                               * sizeof(*word_buf);

/*****************************************************************************/

static void maybe_dump_audio_sample_to_log()
{
#if CONFIG_KWS_DUMP_AUDIO_SAMPLE_TO_LOG
  // arecord -d 1 -f S16_LE -r 16000 | head -c44 | base64
  const char* h = "UklGRiR9AABXQVZFZm10IBAAAAABAAEAgD4AAAB9AAACABAAZGF0YQB9AAA=";
  vTaskDelay(1); // makes task_wdt happy
  size_t o = 0, s = (((4 * WORD_BUF_SZ / 3) + 3) & ~3);
  uint8_t* b = malloc(s + 1); // null terminated
  assert(b);
  mbedtls_base64_encode(b, s + 1,  &o, (uint8_t*)word_buf, WORD_BUF_SZ);
  assert(o == s);
  printf("(echo '%s' | base64 -d; echo '%s' | base64 -d) | aplay -\n", h, b);
  free(b);
  vTaskDelay(1); // makes task_wdt happy
#endif
}

/*****************************************************************************/

void kws_detect_init() 
{
  assert(vad_inst = vad_create(VAD_MODE_4, KWS_SAMPLE_RATE_HZ, KWS_CHUNK_LENGTH_MS));
  ESP_ERROR_CHECK(kws_ring_init(KWS_CHUNKS_IN_WORD, KWS_SAMPLES_IN_CHUNK));
  assert(word_buf = malloc(WORD_BUF_SZ));
}

/*****************************************************************************/

int kws_detect(audio_sample_t* chunk) 
{
  static enum
  {
    STATE_SMOOTHING = -4,
    STATE_QUESSING,
    STATE_RECORDING_WORD,
    STATE_SEEKING_FOR_VOICE,
  }
  state = STATE_SEEKING_FOR_VOICE;

  _Static_assert(STATE_SEEKING_FOR_VOICE == -1, "K.I.S.S.");
  _Static_assert(KWS_CHUNKS_IN_WORD >= (2.5 * CONFIG_KWS_VAD_VOICED_THOLD), "");

  switch(state)
  {
    case STATE_SEEKING_FOR_VOICE:
    case STATE_RECORDING_WORD:
    case STATE_QUESSING:
    case STATE_SMOOTHING:
      break;
    default:
      assert(0);
  }

  switch(state)
  {
    case STATE_SEEKING_FOR_VOICE:
    {
      static int voiced;
      vad_state_t vad_state = vad_process(vad_inst, chunk);

      if(vad_state == VAD_SPEECH)
        voiced++;
      else if(voiced)
        voiced--;

      if(voiced == CONFIG_KWS_VAD_VOICED_THOLD)
      {
        voiced = 0;
        state = STATE_RECORDING_WORD;
      }
      break;
    }
    case STATE_RECORDING_WORD:
    {
      static int count;
      const int remain = KWS_CHUNKS_IN_WORD - (2.5 * CONFIG_KWS_VAD_VOICED_THOLD);
      if(count++ == remain /* experimental */)
      {
        count = 0;
        state = STATE_QUESSING;
      }
      break;
    }
    case STATE_QUESSING:
    {
      audio_sample_t *cursor;
      for(size_t i = 0; i < KWS_CHUNKS_IN_WORD; i++)
      {
        cursor = kws_ring_get_next_chunk();
        for(size_t j = 0; j < KWS_SAMPLES_IN_CHUNK; j++)
          word_buf[(i * KWS_SAMPLES_IN_CHUNK) + j] = cursor[j];
      }
      assert(chunk == cursor);
      maybe_dump_audio_sample_to_log();
      state = STATE_SMOOTHING;
      return kws_guess_one_sec_16b_16k_mono(word_buf);
    }
    case STATE_SMOOTHING:
    {
      static int count;
      const int remain = 2.5 * CONFIG_KWS_VAD_VOICED_THOLD;
      if(count++ == remain /* experimental */)
      {
        count = 0;
        state = STATE_SEEKING_FOR_VOICE;
      }
      break;
    }
  }

  return state;
}

/*****************************************************************************/

int kws_get_samp_rate()
{
  return KWS_SAMPLE_RATE_HZ;
}

/*****************************************************************************/

int kws_get_samp_chunksize()
{
  return KWS_SAMPLES_IN_CHUNK;
}

/*****************************************************************************/

audio_sample_t* kws_get_next_chunk()
{
  return kws_ring_get_next_chunk();
}

/*****************************************************************************/