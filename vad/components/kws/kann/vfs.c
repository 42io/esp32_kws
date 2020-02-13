#include "vfs.h"
#include "esp_log.h"
#include "esp_vfs.h"
#include <string.h>

#if   CONFIG_KWS_MODEL_TYPE_RNN
extern const uint8_t binary_model_start[] asm("_binary_rnn_model_start");
extern const uint8_t binary_model_end[]   asm("_binary_rnn_model_end");
#elif CONFIG_KWS_MODEL_TYPE_CNN
extern const uint8_t binary_model_start[] asm("_binary_cnn_model_start");
extern const uint8_t binary_model_end[]   asm("_binary_cnn_model_end");
#elif CONFIG_KWS_MODEL_TYPE_MLP
extern const uint8_t binary_model_start[] asm("_binary_mlp_model_start");
extern const uint8_t binary_model_end[]   asm("_binary_mlp_model_end");
#endif

static const char* TAG = "kws_fs";

static const uint8_t* binary_model_current;

/*****************************************************************************/

static int kws_fs_open(const char * path, int flags, int mode)
{
    binary_model_current = binary_model_start;
    ESP_LOGI(TAG, "open");
    return 42;
}

/*****************************************************************************/

static int kws_fs_close(int fd)
{
    ESP_LOGI(TAG, "close");
    return 0;
}

/*****************************************************************************/

static int kws_fs_rread (int fd, void * dst, size_t size)
{
    int total = binary_model_end - binary_model_current;
    if (total > size)
        total = size;
    memcpy(dst, binary_model_current, total);
    binary_model_current += total;
    return total;
}

/*****************************************************************************/

void kws_fs_init(const char* name) {
  const esp_vfs_t fs = {
        .flags = ESP_VFS_FLAG_DEFAULT,
        .open = kws_fs_open,
        .close = kws_fs_close,
        .read = kws_fs_rread,
  };

  ESP_ERROR_CHECK(esp_vfs_register(name, &fs, NULL));
}

/*****************************************************************************/