#include "utils.h"

void PUT32(uint32_t location, uint32_t value) {
    asm volatile (
        "str r1, [r0]\n\t"
        "bx lr\n\t"
    );
}

volatile uint32_t GET32(uint32_t location) {
    asm volatile (
        "ldr r0, [r0]\n\t"
        "bx lr\n\t"
    );
}

void put32(volatile uint32_t *addr, uint32_t value) {
    PUT32((uint32_t) addr, value);
}

volatile uint32_t get32(volatile uint32_t *addr) {
    return GET32((uint32_t) addr);
}

void RMW_OR(uint32_t reg, uint32_t mask) {
    PUT32(reg, GET32(reg) | mask);
}

void RMW_AND(uint32_t reg, uint32_t mask) {
    PUT32(reg, GET32(reg) & mask);
}

void interrupts_on(void) {
    asm volatile ("cpsie i");
}

void interrupts_off(void) {
    asm volatile ("cpsid i");
}
