# ESP32 Keyword Spotting | [YouTube](https://youtu.be/GGe0xbQxkhk)

    ~$ git clone --recursive --depth 1 https://github.com/42io/esp32_kws.git && cd esp32_kws

## ESP32 BUILD SDK

	~$ echo '
	     FROM ubuntu:18.04
         RUN apt-get -qq update && \
             apt-get -qq install -y gcc git wget make libncurses-dev flex bison gperf python \
             python-pip python-setuptools python-serial python-cryptography \
             python-future python-pyparsing libffi-dev libssl-dev > /dev/null && \
             mkdir /home/esp && cd /home/esp && \
             wget -q https://dl.espressif.com/dl/xtensa-esp32-elf-linux64-1.22.0-96-g2852398-5.2.0.tar.gz && \
             tar xzf xtensa-esp32-elf-linux64-1.22.0-96-g2852398-5.2.0.tar.gz && \
             rm xtensa-esp32-elf-linux64-1.22.0-96-g2852398-5.2.0.tar.gz && \
             git clone -q -b v3.3.4 --recursive --depth 1 https://github.com/espressif/esp-idf.git
		 ENV PATH /home/esp/xtensa-esp32-elf/bin:$PATH
		 ENV IDF_PATH /home/esp/esp-idf
         WORKDIR /home/src
       ' | docker build --no-cache -t idf-3.3.4 -
	~$ docker run --rm -it -v $PWD:/home/src --device=/dev/ttyUSB0 idf-3.3.4

## Audio -> MFCC -> Neural Network | [Jupyter](../mfcc-nn/components/kws/tf/dcnn.ipynb)

Simple non-streaming mode. Neural network model receives the whole input sequence and then returns the classification result:

    ~$ make -C mfcc-nn defconfig size erase_flash flash monitor
