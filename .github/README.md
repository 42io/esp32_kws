# ESP32 Keyword Spotting

YouTube [EN](https://youtu.be/GGe0xbQxkhk) | [RU](https://youtu.be/OrqWP3F4TGw)

    ~$ git clone --recursive --depth 1 https://github.com/42io/esp32_kws.git
    ~$ cd esp32_kws

#### BUILD ENV

    ~$ docker build --no-cache -t idf-3.3.4 - < Dockerfile
    ~$ docker run --rm -it -v $PWD:/home/src --device=/dev/ttyUSB0 idf-3.3.4

#### KEYWORD SPOTTING

Default models are pre-trained on 0-9 words: zero one two three four five six seven eight nine.

###### MFCC

Simple non-streaming neural network mode. Model receives the whole [mfcc](https://github.com/42io/dataset/tree/master/google_speech_commands#just-for-fun) input sequence and then returns the classification result. [Jupyter](../mfcc-nn/components/kws/tf/dcnn.ipynb):

    ~$ make -C mfcc-nn defconfig size erase_flash flash monitor

###### MFCC STREAMING

[Streaming](https://arxiv.org/abs/2005.06720) neural network mode. Model receives portion of the input sequence and classifies it incrementally. [Jupyter](../mfcc-nn-streaming/components/kws/tf/dcnn.ipynb):

    ~$ make -C mfcc-nn-streaming defconfig size erase_flash flash monitor

###### TRANSFER LEARNING

[Transfer learning](https://arxiv.org/abs/2002.01322) is usually done for tasks where your dataset has too little data to train a full-scale model from scratch. TL consists of taking features learned on one problem, and leveraging them on a new, similar problem. For instance, features from a model that has learned to identify english speech may be useful to kick-start a model meant to identify russian.

[Embedding](../tl/embedding.ipynb) | [Synthesize](https://colab.research.google.com/github/42io/esp32_kws/blob/master/tl/synth.ipynb) | [Transfer](https://colab.research.google.com/github/42io/esp32_kws/blob/master/tl/trans.ipynb)

    ~$ make -C mfcc-nn-streaming menuconfig # kws => ru
    ~$ make -C mfcc-nn-streaming size erase_flash flash monitor
