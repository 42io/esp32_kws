# ESP32 Keyword Spotting

    ~$ git clone --recursive https://github.com/42io/esp32_kws.git && cd esp32_kws

## Real Time (ESP-IDF v3.3.2) [YouTube](https://youtu.be/GGe0xbQxkhk) [Jupyter](../rt/components/kws/tf/dcnn.ipynb)

    ~$ cd rt
    ~$ make defconfig size erase_flash flash monitor

## Vad (ESP-IDF v3.3.1) [YouTube](https://youtu.be/NUfNhjMcK54)

    ~$ cd vad
    ~$ make defconfig size erase_flash flash monitor
    ~$ make menuconfig

![menuconfig](menuconfig-vad.png?raw=true "menuconfig vad")

## Bench (ESP-IDF v3.1)

    ~$ cd bench
    ~$ make defconfig erase_flash flash partition_table
    ~$ URL=https://github.com/igrr/mkspiffs/releases/download/0.2.3/mkspiffs-0.2.3-esp-idf-linux64.tar.gz
    ~$ curl -L $URL | tar zxf - --strip 1
    ~$ ./mkspiffs -c components/kws/c_keyword_spotting/models -b 4096 -p 256 -s 0x400000 /tmp/kws_spiffs.bin
    ~$ alias esptool="$IDF_PATH/components/esptool_py/esptool/esptool.py"
    ~$ esptool --chip esp32 --port /dev/ttyUSB0 --baud 921600 write_flash -z 0x110000 /tmp/kws_spiffs.bin
    ~$ make monitor

![mlp](mlp.png?raw=true "mlp")
![cnn](cnn.png?raw=true "cnn")
![rnn](rnn.png?raw=true "rnn")
