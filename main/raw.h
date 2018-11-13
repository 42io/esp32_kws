#include "assert.h"

int16_t* get_raw_zero()
{
    extern const uint8_t start_zero[] asm("_binary_zero_raw_start");
    extern const uint8_t end_zero[]   asm("_binary_zero_raw_end");
    assert((end_zero - start_zero) == 32000);
    return (int16_t*)&start_zero[0];
}

int16_t* get_raw_unk()
{
    extern const uint8_t start_unk[] asm("_binary_unk_raw_start");
    extern const uint8_t end_unk[]   asm("_binary_unk_raw_end");
    assert((end_unk - start_unk) == 32000);
    return (int16_t*)&start_unk[0];
}

int16_t* get_raw_house()
{
    extern const uint8_t start_house[] asm("_binary_house_raw_start");
    extern const uint8_t end_house[]   asm("_binary_house_raw_end");
    assert((end_house - start_house) == 32000);
    return (int16_t*)&start_house[0];
}

int16_t* get_raw_visual()
{
    extern const uint8_t start_visual[] asm("_binary_visual_raw_start");
    extern const uint8_t end_visual[]   asm("_binary_visual_raw_end");
    assert((end_visual - start_visual) == 32000);
    return (int16_t*)&start_visual[0];
}

int16_t* get_raw_marvin()
{
    extern const uint8_t start_marvin[] asm("_binary_marvin_raw_start");
    extern const uint8_t end_marvin[]   asm("_binary_marvin_raw_end");
    assert((end_marvin - start_marvin) == 32000);
    return (int16_t*)&start_marvin[0];
}