#pragma once

#include <stdint.h>
#include "gpio.h"
#include "timer.h"
#include <stdbool.h>

#define AUX_BASE 0x20215000

#define UART_TX0_PIN 14
#define UART_RX0_PIN 15

typedef enum UART_Num_Bits_t {
    UART_7_BIT,
    UART_8_BIT
} UART_Num_Bits;

typedef struct UART_t {
    UART_Num_Bits num_bits;
    uint32_t baud_rate;
    Pin tx_pin;
    Pin rx_pin;
} UART;

enum {
    AUX_IRQ = AUX_BASE,
    AUX_ENABLES = AUX_BASE + 0x04,
    AUX_MU_IO_REG = AUX_BASE + 0x40,
    AUX_MU_IER_REG = AUX_BASE + 0x44,
    AUX_MU_IIR_REG = AUX_BASE + 0x48,
    AUX_MU_LCR_REG = AUX_BASE + 0x4c,
    AUX_MU_MCR_REG = AUX_BASE + 0x50,
    AUX_MU_LSR_REG = AUX_BASE + 0x54,
    AUX_MU_MSR_REG = AUX_BASE + 0x58,
    AUX_MU_SCRATCH = AUX_BASE + 0x5C,
    AUX_MU_CNTL_REG = AUX_BASE + 0x60,
    AUX_MU_STAT_REG = AUX_BASE + 0x64,
    AUX_MU_BAUD_REG = AUX_BASE + 0x68,
};

void UART_Enable(void);
void UART_Disable(void);
void UART_Config(UART *uart);
uint8_t UART_Receive_Byte(void);
void UART_Send_Byte(const uint8_t byte);
void UART_Send_String(char *str);
void UART_Send_Int(uint32_t num);
void UART_Send_Hex(uint32_t num);
void UART_Print_Int(uint32_t num); // sends int but with a newline
void UART_Print_Hex(uint32_t num); // sends hex but with a newline
void UART_Flush_TX(void);
bool UART_TX_Empty(void);
bool UART_TX_Ready(void);
bool UART_RX_Ready(void);
