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

const Pin bclk = {18};
const Pin fs = {19};
const Pin din = {20};
const Pin dout = {21};

GPCLK pcm_clk = {
    .clk_periph = PCMCLK,
    .source = GPCLK_SRC_PLLD,
    .speed = 2000000
};

I2S i2s = {
    .frame_size = 32,
    .rx_data_size = 24,
    .tx_data_size = 24,
    .bclk = bclk,
    .fs = fs,
    .din = din,
    .dout = dout,
    .clk = &pcm_clk
};
