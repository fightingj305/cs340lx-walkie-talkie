#pragma once
#include <stdint.h>

#define RNG_BASE 0x20104000

enum {
    RNG_CTRL = RNG_BASE + 0x00,
    RNG_STATUS = RNG_BASE + 0x04,
    RNG_DATA = RNG_BASE + 0x08,
    RNG_FF_THRESHOLD = RNG_BASE + 0x0C,
    RNG_INT_MASK = RNG_BASE + 0x010
};


void RNG_Init();
uint32_t RNG_Randint(uint32_t min, uint32_t max);
uint32_t RNG_Random();
void RNG_Off();