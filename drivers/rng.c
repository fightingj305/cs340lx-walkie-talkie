#include "rng.h"
#include "utils.h"


void RNG_Init() {
    PUT32(RNG_INT_MASK, 1);
    PUT32(RNG_STATUS, 0x40000);
    PUT32(RNG_CTRL, 1);
}

uint32_t RNG_Random() {
    while (GET32(RNG_STATUS) >> 24 == 0);
    return GET32(RNG_DATA);
}

uint32_t RNG_Randint(uint32_t min, uint32_t max) {
    ASSERT(max > min);
    return (RNG_Random() % (max - min)) + min;
}

void RNG_Off() {
    PUT32(RNG_CTRL, 0);
}