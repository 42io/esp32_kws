COMPONENT_ADD_INCLUDEDIRS += esp-adf/components/audio_stream/include
COMPONENT_SRCDIRS         += esp-adf/components/audio_stream
COMPONENT_OBJS            += esp-adf/components/audio_stream/raw_stream.o \
                             esp-adf/components/audio_stream/i2s_stream.o

COMPONENT_ADD_INCLUDEDIRS += esp-adf/components/esp-adf-libs/esp_audio/include \
                             esp-adf/components/esp-adf-libs/esp_codec/include/codec \
                             esp-adf/components/esp-adf-libs/esp_codec/include/processing
COMPONENT_SRCDIRS         += esp-adf/components/esp-adf-libs/esp_codec
COMPONENT_OBJS            += esp-adf/components/esp-adf-libs/esp_codec/audio_alc.o \
                             esp-adf/components/esp-adf-libs/esp_codec/filter_resample.o

COMPONENT_ADD_LDFLAGS     += -L$(COMPONENT_PATH)/esp-adf/components/esp-adf-libs/esp_codec/lib/esp32 -lesp_processing