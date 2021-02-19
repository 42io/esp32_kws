COMPONENT_PRIV_INCLUDEDIRS := \
                tf/tfmicro \
                tf/tfmicro/third_party/gemmlowp \
                tf/tfmicro/third_party/flatbuffers/include \
                tf/tfmicro/third_party/ruy \
                fe/src/features \
                fe/lib/c_speech_features-0.4.8 \
                fe/lib/c_speech_features-0.4.8/kiss_fft130

COMPONENT_OBJS := \
                tf/guess.o \
                tf/tfmicro/tensorflow/lite/micro/simple_memory_allocator.o \
                tf/tfmicro/tensorflow/lite/micro/all_ops_resolver.o \
                tf/tfmicro/tensorflow/lite/micro/memory_helpers.o \
                tf/tfmicro/tensorflow/lite/micro/recording_micro_allocator.o \
                tf/tfmicro/tensorflow/lite/micro/micro_error_reporter.o \
                tf/tfmicro/tensorflow/lite/micro/micro_time.o \
                tf/tfmicro/tensorflow/lite/micro/recording_simple_memory_allocator.o \
                tf/tfmicro/tensorflow/lite/micro/micro_string.o \
                tf/tfmicro/tensorflow/lite/micro/micro_profiler.o \
                tf/tfmicro/tensorflow/lite/micro/micro_utils.o \
                tf/tfmicro/tensorflow/lite/micro/micro_optional_debug_tools.o \
                tf/tfmicro/tensorflow/lite/micro/debug_log.o \
                tf/tfmicro/tensorflow/lite/micro/test_helpers.o \
                tf/tfmicro/tensorflow/lite/micro/micro_interpreter.o \
                tf/tfmicro/tensorflow/lite/micro/micro_allocator.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/depthwise_conv.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/split.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/conv.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/strided_slice.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/concatenation.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/reshape.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/dequantize.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/prelu.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/pad.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/l2norm.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/tanh.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/resize_nearest_neighbor.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/pack.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/activations.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/ceil.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/arg_min_max.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/reduce.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/fully_connected.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/add.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/floor.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/circular_buffer.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/softmax.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/ethosu.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/sub.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/unpack.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/round.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/neg.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/quantize.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/elementwise.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/logistic.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/svdf.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/maximum_minimum.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/pooling.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/logical.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/mul.o \
                tf/tfmicro/tensorflow/lite/micro/kernels/comparisons.o \
                tf/tfmicro/tensorflow/lite/micro/memory_planner/linear_memory_planner.o \
                tf/tfmicro/tensorflow/lite/micro/memory_planner/greedy_memory_planner.o \
                tf/tfmicro/tensorflow/lite/c/common.o \
                tf/tfmicro/tensorflow/lite/core/api/error_reporter.o \
                tf/tfmicro/tensorflow/lite/core/api/flatbuffer_conversions.o \
                tf/tfmicro/tensorflow/lite/core/api/op_resolver.o \
                tf/tfmicro/tensorflow/lite/core/api/tensor_utils.o \
                tf/tfmicro/tensorflow/lite/kernels/internal/quantization_util.o \
                tf/tfmicro/tensorflow/lite/kernels/kernel_util.o \
                fe/src/features/fe.o \
                fe/src/features/fast/mfcc.o \
                fe/lib/c_speech_features-0.4.8/slow/fbank.o \
                fe/lib/c_speech_features-0.4.8/slow/logfbank.o \
                fe/lib/c_speech_features-0.4.8/slow/lifter.o \
                fe/lib/c_speech_features-0.4.8/slow/get_filterbanks.o \
                fe/lib/c_speech_features-0.4.8/slow/framesig.o \
                fe/lib/c_speech_features-0.4.8/slow/preemphasis.o \
                fe/src/features/fast/magspec.o \
                fe/lib/c_speech_features-0.4.8/slow/powspec.o \
                fe/lib/c_speech_features-0.4.8/kiss_fft130/kiss_fft.o \
                fe/lib/c_speech_features-0.4.8/kiss_fft130/tools/kiss_fftr.o

COMPONENT_SRCDIRS := \
                tf \
                tf/tfmicro/tensorflow/lite/micro \
                tf/tfmicro/tensorflow/lite/micro/kernels \
                tf/tfmicro/tensorflow/lite/micro/memory_planner \
                tf/tfmicro/tensorflow/lite/c \
                tf/tfmicro/tensorflow/lite/core/api \
                tf/tfmicro/tensorflow/lite/kernels/internal \
                tf/tfmicro/tensorflow/lite/kernels \
                fe/src/features \
                fe/src/features/fast \
                fe/lib/c_speech_features-0.4.8/slow \
                fe/lib/c_speech_features-0.4.8/kiss_fft130 \
                fe/lib/c_speech_features-0.4.8/kiss_fft130/tools

tf/tfmicro/tensorflow/lite/micro/micro_interpreter.o: CXXFLAGS += -Wno-type-limits
tf/tfmicro/tensorflow/lite/micro/micro_string.o: CXXFLAGS += -Wno-strict-aliasing
tf/tfmicro/tensorflow/lite/micro/micro_allocator.o: CXXFLAGS += -Wno-missing-field-initializers
fe/lib/c_speech_features-0.4.8/slow/fbank.o: CFLAGS += -Wno-maybe-uninitialized

CFLAGS += -DTF_LITE_STATIC_MEMORY
CXXFLAGS += -O3 -DNDEBUG -std=c++11 -DTF_LITE_STATIC_MEMORY -fno-rtti

COMPONENT_EMBED_FILES := tf/dcnn.en.tflite tf/dcnn.ru.tflite

COMPONENT_PRIV_INCLUDEDIRS += .

COMPONENT_SRCDIRS += .

COMPONENT_OBJS += kws.o

CFLAGS += -Werror -Wall -Wextra
CXXFLAGS += -Werror -Wall -Wextra