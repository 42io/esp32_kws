# ESP32 Keyword Spotting

    ~: git clone --recursive https://github.com/42io/esp32_kws.git && cd esp32_kws
    ~: make erase_flash flash partition_table
    ~: ./mkspiffs -c components/kws/c_keyword_spotting/models -b 4096 -p 256 -s 0x400000 /tmp/kws_spiffs.bin
    ~: $IDF_PATH/components/esptool_py/esptool/esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 921600 write_flash -z 0x110000 /tmp/kws_spiffs.bin
    ~: make monitor

![mlp](mlp.png?raw=true "mlp")
