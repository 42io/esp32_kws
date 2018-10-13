/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "esp_system.h"
#include "esp_timer.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "raw.h"
#include "kws.h"
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char LOG_TAG[] = "[main]";

static void load_kws_model(const char* name)
{
    ESP_LOGW(LOG_TAG, "Loading kws, free heap %d bytes", esp_get_free_heap_size());
    int64_t t1 = esp_timer_get_time();
    kws_init(name);
    ESP_LOGW(LOG_TAG,"kws %s load time %f s, free heap %d bytes",
        name,
        ((float)(esp_timer_get_time() - t1))/1000000,
        esp_get_free_heap_size());
}

static void assert_floats_equals(float a, float b)
{
    assert(fabs(a - b) < 0.000001);
}

static void test_guess(int16_t* samples, float f_house, float f_zero, float f_unk)
{
    int64_t t1 = esp_timer_get_time();
    const float* guess = kws_guess_one_sec_16b_16k_mono(samples);
    ESP_LOGW(LOG_TAG,"kws guess time %f s, free heap %d bytes",
        ((float)(esp_timer_get_time() - t1))/1000000,
        esp_get_free_heap_size());
    assert_floats_equals(guess[0], f_house);
    assert_floats_equals(guess[1], f_zero);
    assert_floats_equals(guess[2], f_unk);
}

void app_main()
{
    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    ESP_LOGI(LOG_TAG,
            "This is ESP32 chip with %d CPU cores, WiFi%s%s, "
            "silicon revision %d, "
            "%dMB %s flash",
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "",
            chip_info.revision,
            spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    while(42)
    {
        load_kws_model("/spiffs/mlp.model");
        test_guess(get_raw_house(), 0.996505, 0, 0.002565);
        test_guess(get_raw_zero(), 0, 0.997143, 0.001153);
        test_guess(get_raw_unk(), 0.000318, 0.001399, 0.996432);
        kws_deinit();

        // TODO: cnn requres more then 4MB RAM

        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}
