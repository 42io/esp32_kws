#include "assert.h"

int16_t* get_raw_zero()
{
    extern const uint8_t zero_raw_start[] asm("_binary_zero_raw_start");
    extern const uint8_t zero_raw_end[]   asm("_binary_zero_raw_end");
    assert((zero_raw_end - zero_raw_start) == 32000);
    return (int16_t*)&zero_raw_start[0];
}

int16_t* get_raw_unk()
{
    extern const uint8_t unk_raw_start[] asm("_binary_unk_raw_start");
    extern const uint8_t unk_raw_end[]   asm("_binary_unk_raw_end");
    assert((unk_raw_end - unk_raw_start) == 32000);
    return (int16_t*)&unk_raw_start[0];
}

int16_t* get_raw_house()
{
    extern const uint8_t house_start[] asm("_binary_house_raw_start");
    extern const uint8_t house_end[]   asm("_binary_house_raw_end");
    assert((house_end - house_start) == 32000);
    return (int16_t*)&house_start[0];
}