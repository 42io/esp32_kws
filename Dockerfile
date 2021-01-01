FROM ubuntu:18.04
ARG TOOL=xtensa-esp32-elf-linux64-1.22.0-96-g2852398-5.2.0.tar.gz
RUN apt-get -qq update && \
	apt-get -qq install -y gcc git wget make libncurses-dev \
	flex bison gperf python python-pip python-setuptools \
	python-serial python-cryptography python-future \
	python-pyparsing libffi-dev libssl-dev > /dev/null && \
	mkdir /home/esp && cd /home/esp && \
	wget -q https://dl.espressif.com/dl/$TOOL && \
	tar xzf $TOOL && rm $TOOL && \
	git clone -q -b v3.3.4 --recursive --depth 1 \
	https://github.com/espressif/esp-idf.git
ENV PATH /home/esp/xtensa-esp32-elf/bin:$PATH
ENV IDF_PATH /home/esp/esp-idf
WORKDIR /home/src