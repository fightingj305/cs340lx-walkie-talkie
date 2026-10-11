#pragma once

#include <stdint.h>
#include "gpio.h"
#include <stdbool.h>

#define UART_DELAY 2170

enum {
    UART_TX_PIN = 14,
    UART_RX_PIN = 21,
    UART_START_BIT = 0,
    UART_STOP_BIT = 9,
};

typedef struct UART_t {
    uint8_t UART_TX_byte;
    volatile uint8_t UART_TX_bit;
    Pin UART_TX_Pin;
} UART;

typedef struct UART_RX_t {
    uint8_t UART_RX_byte;
    volatile uint8_t UART_RX_bit;
    Pin UART_RX_Pin;
    uint32_t num_received;
} UART_RX;


void UART_SendByte(UART *uart, const uint8_t byte);
void UART_SendBytes(UART *uart, const uint8_t *byte, uint32_t num_bytes);
void UART_TimerHandler(UART *uart);
void UART_Timer_Setup(UART *uart);
void UART_Receiver_Setup(UART_RX *uart);
void UART_RXTimerHandler(UART_RX *uart);

