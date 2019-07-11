COMPONENT_PRIV_INCLUDEDIRS := \
                c_keyword_spotting/lib/kann-master \
                c_keyword_spotting/src/features \
                c_keyword_spotting/lib/c_speech_features-0.4.8 \
                c_keyword_spotting/lib/c_speech_features-0.4.8/kiss_fft130 \
                c_keyword_spotting/lib/c_speech_features-0.4.8/kiss_fft130/tools

COMPONENT_OBJS := \
                guess.o \
                kws_vfs.o \
                c_keyword_spotting/lib/kann-master/kann.o \
                c_keyword_spotting/lib/kann-master/kautodiff.o \
                c_keyword_spotting/src/features/fe.o \
                c_keyword_spotting/lib/c_speech_features-0.4.8/c_speech_features.o \
                c_keyword_spotting/lib/c_speech_features-0.4.8/kiss_fft130/kiss_fft.o \
                c_keyword_spotting/lib/c_speech_features-0.4.8/kiss_fft130/tools/kiss_fftr.o

COMPONENT_SRCDIRS := . \
                c_keyword_spotting/lib/kann-master \
                c_keyword_spotting/src/features \
                c_keyword_spotting/lib/c_speech_features-0.4.8 \
                c_keyword_spotting/lib/c_speech_features-0.4.8/kiss_fft130 \
                c_keyword_spotting/lib/c_speech_features-0.4.8/kiss_fft130/tools

c_keyword_spotting/src/features/fe.o: CFLAGS += -Wno-maybe-uninitialized
c_keyword_spotting/lib/c_speech_features-0.4.8/c_speech_features.o: CFLAGS += -Wno-maybe-uninitialized

CFLAGS += -Werror -Wall -Wextra