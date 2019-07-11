#include "kws_priv.h"
#include "fe.h"
#include "kann.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

static const char LOG_TAG[] = "kws_guess";

static kann_t *g_ann;
static bool g_is_rnn;

/*****************************************************************************/

void kws_init(const char* name)
{
    assert(sizeof(float) == 4);
    kws_fs_init(name);
    g_ann = kann_load(name);
    assert(g_ann);
    g_is_rnn = kann_is_rnn(g_ann);
    kws_detect_init();
}

/*****************************************************************************/

static float* kws_fe_one_sec_16b_16k_mono(audio_sample_t samples[16000])
{
    int n_frames, n_items_in_frame;
    csf_float *feat;

    n_frames = n_items_in_frame = 0;
    feat = fe_mfcc_16k_16b_mono(samples, 16000, &n_frames, &n_items_in_frame);
    assert(n_frames == 49);
    assert(n_items_in_frame == 13);
    assert(n_items_in_frame > 0);
    assert(feat);

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

static const float* kws_guess_fe(float* feat)
{
    const float* out = NULL;

    if(g_is_rnn)
    {
        assert(kann_dim_in(g_ann) == 13);
        kann_rnn_start(g_ann);
        for(int k = 0; k < 49; k++)
        {
            out = kann_apply1(g_ann, &feat[k*13]);
        }
        kann_rnn_end(g_ann);
    }
    else
    {
        assert(kann_dim_in(g_ann) == 13*49);
        out = kann_apply1(g_ann, feat);
    }

    assert(out);

    // for (int i = 0; i < kann_dim_out(g_ann); i++)
    // {
    //     if (i)
    //     {
    //         putchar(' ');
    //     }
    //     printf("%f", out[i]);
    // }
    // putchar('\n');

    return out;
}

/*****************************************************************************/

static int vec_to_idx(const float a[])
{
  int i, idx = -1;
  float max;
  for(i = 0; i < kann_dim_out(g_ann); i++)
  {
    if(i == 0 || a[i] > max)
    {
      max = a[i];
      idx = i;
    }
  }
  assert(idx >= 0);
  return idx;
}

/*****************************************************************************/

int kws_guess_one_sec_16b_16k_mono(audio_sample_t* samples)
{
    float* feat = kws_fe_one_sec_16b_16k_mono(samples);
    const float* out = kws_guess_fe(feat);
    int idx = vec_to_idx(out);
    free(feat);
    return idx;
}

/*****************************************************************************/