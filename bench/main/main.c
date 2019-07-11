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
    assert(fabs(a - b) < 0.0002);
}

static void test_guess(int16_t* samples, float f_hou, float f_zer, float f_mar,  float f_vis, float f_unk)
{
    int64_t t1 = esp_timer_get_time();
    const float* guess = kws_guess_one_sec_16b_16k_mono(samples);
    ESP_LOGW(LOG_TAG,"kws guess time %f s, free heap %d bytes",
        ((float)(esp_timer_get_time() - t1))/1000000,
        esp_get_free_heap_size());
    assert_floats_equals(guess[0], f_hou);
    assert_floats_equals(guess[1], f_zer);
    assert_floats_equals(guess[2], f_mar);
    assert_floats_equals(guess[3], f_vis);
    assert_floats_equals(guess[4], f_unk);
    vTaskDelay(1); // make WDT happy
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
        test_guess(get_raw_house(),  1.000000, 0.000000, 0.000000, 0.000000, 0.000000);
        test_guess(get_raw_zero(),   0.000000, 0.815244, 0.000000, 0.004613, 0.000102);
        test_guess(get_raw_marvin(), 0.000000, 0.000000, 0.996841, 0.000000, 0.002811);
        test_guess(get_raw_visual(), 0.000000, 0.114680, 0.000000, 0.999759, 0.000000);
        test_guess(get_raw_unk(),    0.000254, 0.000000, 0.032188, 0.000053, 0.898670);
        kws_deinit();

        load_kws_model("/spiffs/cnn.model");
        test_guess(get_raw_house(),  0.999965, 0.000000, 0.000000, 0.000000, 0.000004);
        test_guess(get_raw_zero(),   0.000000, 0.998417, 0.000000, 0.000150, 0.000000);
        test_guess(get_raw_marvin(), 0.000000, 0.000000, 0.999837, 0.000058, 0.000000);
        test_guess(get_raw_visual(), 0.000000, 0.002439, 0.000000, 0.998520, 0.000000);
        test_guess(get_raw_unk(),    0.000554, 0.024503, 0.112318, 0.064628, 0.233033);
        kws_deinit();

        load_kws_model("/spiffs/rnn.model");
        test_guess(get_raw_house(),  0.999991, 0.000002, 0.000004, 0.000001, 0.000013);
        test_guess(get_raw_zero(),   0.000000, 0.751828, 0.000002, 0.333919, 0.000010);
        test_guess(get_raw_marvin(), 0.000001, 0.000002, 0.999962, 0.000002, 0.000012);
        test_guess(get_raw_visual(), 0.000020, 0.000025, 0.000001, 0.999986, 0.000003);
        test_guess(get_raw_unk(),    0.000013, 0.000090, 0.000040, 0.001414, 0.999798);
        kws_deinit();

        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}
