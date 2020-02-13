ifdef CONFIG_KWS_PLATFORM_TYPE_TF

COMPONENT_PRIV_INCLUDEDIRS := \
                tf/tfmicro \
                tf/tfmicro/third_party/gemmlowp \
                tf/tfmicro/third_party/flatbuffers/include \
                tf/tflite_kws/dataset/dataset/google_speech_commands/src/features \
                tf/tflite_kws/dataset/dataset/google_speech_commands/lib/c_speech_features-0.4.8 \
                tf/tflite_kws/dataset/dataset/google_speech_commands/lib/c_speech_features-0.4.8/kiss_fft130 \
                tf/tflite_kws/dataset/dataset/google_speech_commands/lib/c_speech_features-0.4.8/kiss_fft130/tools

COMPONENT_OBJS := \
                tf/kws.o \
                tf/tfmicro/tensorflow/lite/micro/simple_memory_allocator.o \
                tf/tfmicro/tensorflow/lite/micro/memory_helpers.o \
                tf/tfmicro/tensorflow/lite/micro/test_helpers.o \
                tf/tfmicro/tensorflow/lite/micro/micro_utils.o \
                tf/tfmicro/tensorflow/lite/micro/micro_error_reporter.o \
                tf/tfmicro/tensorflow/lite/micro/micro_allocator.o \
                tf/tfmicro/tensorflow/lite/micro/micro_optional_debug_tools.o \
                tf/tfmicro/tensorflow/lite/micro/debug_log.o \
                tf/tfmicro/tensorflow/lite/micro/debug_log_numbers.o \
                tf/tfmicro/tensorflow/lite/micro/micro_interpreter.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/pack.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/logical.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/comparisons.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/depthwise_conv.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/split.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/pooling.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/prelu.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/dequantize.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/activations.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/arg_min_max.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/pad.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/reduce.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/conv.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/fully_connected.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/add.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/ceil.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/floor.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/softmax.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/logistic.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/svdf.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/concatenation.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/unpack.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/round.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/quantize.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/elementwise.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/all_ops_resolver.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/mul.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/maximum_minimum.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/reshape.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/strided_slice.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/neg.o \
                tf/tfmicro/tensorflow/lite/micro/memory_planner/linear_memory_planner.o \
                tf/tfmicro/tensorflow/lite/micro/memory_planner/greedy_memory_planner.o \
                tf/tfmicro/tensorflow/lite/c/common.o \
                tf/tfmicro/tensorflow/lite/core/api/error_reporter.o \
                tf/tfmicro/tensorflow/lite/core/api/flatbuffer_conversions.o \
                tf/tfmicro/tensorflow/lite/core/api/op_resolver.o \
                tf/tfmicro/tensorflow/lite/core/api/tensor_utils.o \
                tf/tfmicro/tensorflow/lite/kernels/internal/quantization_util.o \
                tf/tfmicro/tensorflow/lite/kernels/kernel_util.o \
                tf/tflite_kws/dataset/dataset/google_speech_commands/src/features/fe.o \
                tf/tflite_kws/dataset/dataset/google_speech_commands/lib/c_speech_features-0.4.8/c_speech_features.o \
                tf/tflite_kws/dataset/dataset/google_speech_commands/lib/c_speech_features-0.4.8/kiss_fft130/kiss_fft.o \
                tf/tflite_kws/dataset/dataset/google_speech_commands/lib/c_speech_features-0.4.8/kiss_fft130/tools/kiss_fftr.o

COMPONENT_SRCDIRS := \
                tf \
                tf/tfmicro/tensorflow/lite/micro \
                tf/tfmicro/tensorflow/lite/micro/kernels \
                tf/tfmicro/tensorflow/lite/micro/memory_planner \
                tf/tfmicro/tensorflow/lite/c \
                tf/tfmicro/tensorflow/lite/core/api \
                tf/tfmicro/tensorflow/lite/kernels/internal \
                tf/tfmicro/tensorflow/lite/kernels \
                tf/tflite_kws/dataset/dataset/google_speech_commands/src/features \
                tf/tflite_kws/dataset/dataset/google_speech_commands/lib/c_speech_features-0.4.8 \
                tf/tflite_kws/dataset/dataset/google_speech_commands/lib/c_speech_features-0.4.8/kiss_fft130 \
                tf/tflite_kws/dataset/dataset/google_speech_commands/lib/c_speech_features-0.4.8/kiss_fft130/tools

tf/tfmicro/tensorflow/lite/micro/micro_interpreter.o: CXXFLAGS += -Wno-strict-aliasing -Wno-type-limits
tf/tfmicro/tensorflow/lite/micro/debug_log_numbers.o: CXXFLAGS += -Wno-strict-aliasing
tf/tflite_kws/dataset/dataset/google_speech_commands/src/features/fe.o: CFLAGS += -Wno-maybe-uninitialized
tf/tflite_kws/dataset/dataset/google_speech_commands/lib/c_speech_features-0.4.8/c_speech_features.o: CFLAGS += -Wno-maybe-uninitialized

CFLAGS += -DTF_LITE_STATIC_MEMORY
CXXFLAGS += -O3 -DNDEBUG -std=c++11 -DTF_LITE_STATIC_MEMORY -fno-rtti

COMPONENT_EMBED_FILES := tf/tflite_kws/models/dcnn.tflite \
                         tf/tflite_kws/models/cnn.tflite \
                         tf/tflite_kws/models/mlp.tflite

endif

ifdef CONFIG_KWS_PLATFORM_TYPE_KANN

COMPONENT_PRIV_INCLUDEDIRS := \
                kann/c_keyword_spotting/lib/kann-master \
                kann/c_keyword_spotting/src/features \
                kann/c_keyword_spotting/lib/c_speech_features-0.4.8 \
                kann/c_keyword_spotting/lib/c_speech_features-0.4.8/kiss_fft130 \
                kann/c_keyword_spotting/lib/c_speech_features-0.4.8/kiss_fft130/tools

COMPONENT_OBJS := \
                kann/kws.o \
                kann/vfs.o \
                kann/c_keyword_spotting/lib/kann-master/kann.o \
                kann/c_keyword_spotting/lib/kann-master/kautodiff.o \
                kann/c_keyword_spotting/src/features/fe.o \
                kann/c_keyword_spotting/lib/c_speech_features-0.4.8/c_speech_features.o \
                kann/c_keyword_spotting/lib/c_speech_features-0.4.8/kiss_fft130/kiss_fft.o \
                kann/c_keyword_spotting/lib/c_speech_features-0.4.8/kiss_fft130/tools/kiss_fftr.o

COMPONENT_SRCDIRS := \
                kann \
                kann/c_keyword_spotting/lib/kann-master \
                kann/c_keyword_spotting/src/features \
                kann/c_keyword_spotting/lib/c_speech_features-0.4.8 \
                kann/c_keyword_spotting/lib/c_speech_features-0.4.8/kiss_fft130 \
                kann/c_keyword_spotting/lib/c_speech_features-0.4.8/kiss_fft130/tools

kann/c_keyword_spotting/src/features/fe.o: CFLAGS += -Wno-maybe-uninitialized
kann/c_keyword_spotting/lib/c_speech_features-0.4.8/c_speech_features.o: CFLAGS += -Wno-maybe-uninitialized

COMPONENT_EMBED_FILES := kann/c_keyword_spotting/models/rnn.model \
                         kann/c_keyword_spotting/models/cnn.model \
                         kann/c_keyword_spotting/models/mlp.model

endif

COMPONENT_PRIV_INCLUDEDIRS += .

COMPONENT_SRCDIRS += .

COMPONENT_OBJS += \
                detect.o \
                ring.o

CFLAGS += -Werror -Wall -Wextra
CXXFLAGS += -Werror -Wall -Wextra