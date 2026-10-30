#pragma once

#include <stdint.h>
#include "miniuart.h"

#define BCM_PERIPH_BASE     0x20000000
#define BCM_PERIPH_BUS_BASE 0x7E000000
#define BCM_RAM_BUS         0x40000000

#define CORE_CLOCK_RATE 250000000
#define ARM_CLOCK_RATE 700000000

#define CYCLES_PER_US (ARM_CLOCK_RATE / 1000000UL)  // 700
#define CYCLES_PER_MS (ARM_CLOCK_RATE / 1000UL)     // 700000

#define ASSERT(x) \
    do { \
        if(!(x)) PANIC("Assertion failed: " #x); \
    } while(0)

#define DELAY(cycles) do { \
        for (volatile uint32_t i = (cycles); i > 0; i--); \
    } while(0)

#define DELAY_US(x) DELAY((x) * CYCLES_PER_US)

#define DELAY_MS(x) DELAY((x) * CYCLES_PER_MS)

#define PANIC(msg) do { \
        UART_Send_String("PANIC: "); \
        UART_Send_String(msg); \
        UART_Send_String(" at "); \
        UART_Send_String(__FILE__); \
        UART_Send_String(":"); \
        UART_Send_Int(__LINE__); \
        UART_Send_String("\n"); \
        while(1); \
    } while(0)

__attribute__((naked))
void PUT32(uint32_t location, uint32_t value);

__attribute__((naked))
volatile uint32_t GET32(uint32_t location);

void put32(volatile uint32_t *addr, uint32_t value);

volatile uint32_t get32(volatile uint32_t *addr);

void RMW_OR(uint32_t reg, uint32_t mask);
void RMW_AND(uint32_t reg, uint32_t mask);

void interrupts_on(void);
void interrupts_off(void);


static inline void ISB(void) {
    asm volatile("mcr p15, 0, %0, c7, c5, 4" :: "r"(0) : "memory");\
}

static inline void DSB(void) {
    asm volatile("mcr p15, 0, %0, c7, c10, 4" :: "r"(0) : "memory");
    ISB();
}

static inline void DMB(void) {
    asm volatile("mcr p15, 0, %0, c7, c10, 5" :: "r"(0) : "memory");
    ISB();
}

static inline uint32_t mem_to_bus(uint32_t addr) {
    if (addr >= BCM_PERIPH_BASE && addr < (BCM_PERIPH_BASE + 0x01000000)) {
        return (addr - BCM_PERIPH_BASE) + BCM_PERIPH_BUS_BASE;
    } else {
        return addr | BCM_RAM_BUS;
    }
}
static inline uint32_t bus_to_mem(uint32_t addr) {
     if (addr >= BCM_PERIPH_BUS_BASE && addr < (BCM_PERIPH_BUS_BASE + 0x01000000)) {
        return (addr - BCM_PERIPH_BUS_BASE) + BCM_PERIPH_BASE;
    } else {
        return addr & ~BCM_RAM_BUS;
    }
}