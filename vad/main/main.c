/* Examples of speech recognition with multiple keywords.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "board.h"
#include "audio_common.h"
#include "audio_pipeline.h"
#include "i2s_stream.h"
#include "raw_stream.h"
#include "filter_resample.h"
#include "periph_is31fl3216.h"
#include "kws.h"

static const char *TAG = "main";

void app_main()
{
    esp_log_level_set("*", ESP_LOG_INFO);

    ESP_LOGI(TAG, "Initialize SR handle");
    kws_init();
    int sample_rate = kws_get_samp_rate();
    int audio_chunksize = kws_get_samp_chunksize();

    ESP_LOGI(TAG, "[ 1 ] Start codec chip");
    audio_board_handle_t board_handle = audio_board_init();
    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_BOTH, AUDIO_HAL_CTRL_START);

    audio_pipeline_handle_t pipeline;
    audio_element_handle_t i2s_stream_reader, filter, raw_read;

    ESP_LOGI(TAG, "[ 2.0 ] Create audio pipeline for recording");
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);
    mem_assert(pipeline);

    ESP_LOGI(TAG, "[ 2.1 ] Create i2s stream to read audio data from codec chip");
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.i2s_config.sample_rate = 48000;
    i2s_cfg.type = AUDIO_STREAM_READER;
    i2s_stream_reader = i2s_stream_init(&i2s_cfg);

    ESP_LOGI(TAG, "[ 2.2 ] Create filter to resample audio data");
    rsp_filter_cfg_t rsp_cfg = DEFAULT_RESAMPLE_FILTER_CONFIG();
    rsp_cfg.src_rate = 48000;
    rsp_cfg.src_ch = 2;
    rsp_cfg.dest_rate = sample_rate;
    rsp_cfg.dest_ch = 1;
    rsp_cfg.sample_bits = 8 * sizeof(audio_sample_t);
    rsp_cfg.type = AUDIO_CODEC_TYPE_ENCODER;
    filter = rsp_filter_init(&rsp_cfg);

    ESP_LOGI(TAG, "[ 2.3 ] Create raw to receive data");
    raw_stream_cfg_t raw_cfg = {
        .out_rb_size = 8 * 1024,
        .type = AUDIO_STREAM_READER,
    };
    raw_read = raw_stream_init(&raw_cfg);

    ESP_LOGI(TAG, "[ 3 ] Register all elements to audio pipeline");
    audio_pipeline_register(pipeline, i2s_stream_reader, "i2s");
    audio_pipeline_register(pipeline, filter, "filter");
    audio_pipeline_register(pipeline, raw_read, "raw");

    ESP_LOGI(TAG, "[ 4 ] Link elements together [codec_chip]-->i2s_stream-->filter-->raw-->[SR]");
    audio_pipeline_link(pipeline, (const char *[]) {"i2s", "filter", "raw"}, 3);

    ESP_LOGI(TAG, "[ 5 ] Start audio_pipeline");
    audio_pipeline_run(pipeline);

    ESP_LOGI(TAG, "[ 6 ] Initialize peripherals");
    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    esp_periph_set_handle_t set = esp_periph_set_init(&periph_cfg);

    ESP_LOGI(TAG, "[ 6.1 ] Initialize IS31fl3216 peripheral");
    periph_is31fl3216_cfg_t is31fl3216_cfg = { 0 };
    esp_periph_handle_t is31fl3216_periph = periph_is31fl3216_init(&is31fl3216_cfg);

    ESP_LOGI(TAG, "[ 6.2 ] Start peripherals");
    esp_periph_start(set, is31fl3216_periph);

    ESP_LOGI(TAG, "[ 6.3 ] Set duty for each LED index");

    periph_is31fl3216_set_blink_pattern(is31fl3216_periph, 0xFFFF);
    periph_is31fl3216_set_state(is31fl3216_periph, IS31FL3216_STATE_ON);

    while (1) {
        audio_sample_t* chunk = kws_get_next_chunk();
        raw_stream_read(raw_read, (char *)chunk, audio_chunksize * sizeof(audio_sample_t));
        int keyword = kws_detect(chunk);
        if(keyword > -1)
        {
            ESP_LOGI(TAG, "Word Detected %d; Memory Total:%d Bytes, SPI:%d Bytes, Inter:%d Bytes, Dram:%d Bytes",
                     keyword, esp_get_free_heap_size(),
                     heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
                     heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
                     heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
            if(keyword < 10)
            {
                for (int i = 0; i < 10; i++)
                {
                    periph_is31fl3216_set_duty(is31fl3216_periph, i, i < keyword ? 255 : 0);
                }
            }
        }
        else
        {
            static bool scanning = true;
            if(scanning == (keyword == -1))
            {
                if(scanning)
                {
                    periph_is31fl3216_set_duty(is31fl3216_periph, 13, 255);
                    periph_is31fl3216_set_duty(is31fl3216_periph, 12, 0);
                    ESP_LOGI(TAG, "Scanning for human word...");
                }
                else
                {
                    periph_is31fl3216_set_duty(is31fl3216_periph, 12, 255);
                    periph_is31fl3216_set_duty(is31fl3216_periph, 13, 0);
                    ESP_LOGI(TAG, "Recognizing...");
                }
                scanning = !scanning;
            }
        }
    }

    ESP_LOGI(TAG, "[ 6 ] Stop audio_pipeline");

    audio_pipeline_terminate(pipeline);

    /* Terminate the pipeline before removing the listener */
    audio_pipeline_remove_listener(pipeline);

    audio_pipeline_unregister(pipeline, raw_read);
    audio_pipeline_unregister(pipeline, i2s_stream_reader);
    audio_pipeline_unregister(pipeline, filter);

    /* Release all resources */
    audio_pipeline_deinit(pipeline);
    audio_element_deinit(raw_read);
    audio_element_deinit(i2s_stream_reader);
    audio_element_deinit(filter);
}
