#include "hardware.h"

const Pin tx = {14};
const Pin rx = {15};

UART uart = {
    .tx_pin = tx,
    .rx_pin = rx,
    .num_bits = UART_8_BIT,
    .baud_rate = 115200
};

const Pin mosi = {10};
const Pin miso = {9};
const Pin sclk = {11};
const Pin cs0 = {8};
const Pin cs1 = {7};

const SPI spi = {
    .speed = 20000000, // 20 MHz
    .cpol = CPOL_0,
    .cpha = CPHA_0,
    .msb_first = true,

    .mosi = mosi,
    .miso = miso,
    .sclk = sclk,
    .cs0 = cs0,
    .cs1 = cs1
};
