#include "kws_priv.h"

/*****************************************************************************/

static struct
{
  size_t chunks_cursor, chunks_cnt, samples_cnt_in_chunk;
  audio_sample_t *samples;
} *ring;

/*****************************************************************************/

esp_err_t kws_ring_init(size_t chunks_cnt, size_t samples_cnt_in_chunk)
{
  if(!chunks_cnt || !samples_cnt_in_chunk)
  {
    return ESP_ERR_INVALID_ARG;
  }

  ring = malloc(sizeof *ring);
  if(ring == NULL)
  {
    return ESP_ERR_NO_MEM;
  }

  ring->samples = calloc(chunks_cnt * samples_cnt_in_chunk, sizeof *ring->samples);
  if(ring->samples == NULL)
  {
    free(ring);
    return ESP_ERR_NO_MEM;
  }

  ring->samples_cnt_in_chunk = samples_cnt_in_chunk;
  ring->chunks_cnt = chunks_cnt;
  ring->chunks_cursor = 0;

  return ESP_OK;
}

/*****************************************************************************/

void kws_ring_free()
{
  free(ring->samples);
  free(ring);
}

/*****************************************************************************/

audio_sample_t *kws_ring_get_next_chunk()
{
  size_t offset = ring->chunks_cursor * ring->samples_cnt_in_chunk;
  audio_sample_t *frame = &ring->samples[offset];
  if(++ring->chunks_cursor == ring->chunks_cnt)
  {
    ring->chunks_cursor = 0;
  }
  return frame;
}

/*****************************************************************************/