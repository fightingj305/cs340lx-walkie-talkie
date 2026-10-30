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

// previous version:
// const SPI spi = {
//     .speed = 20000000, // 20 MHz
//
// why the change: the cc1101 maxes out at 6.5 MHz for back-to-back bytes (datasheet table 22), and
// the pi's SPI0 sends bytes with no gap, so 20 MHz is out of spec. 1 MHz is plenty for bring-up.
// const dropped because SPI_Config takes a non-const SPI * and -Werror rejects passing &spi.
SPI spi = {
    .speed = 20000000,
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

// GPCLK pcm_clk = {
//     .clk_periph = PCMCLK,
//     .source = GPCLK_SRC_PLLD,
//     .speed = 48000 * 32 * 2
// };

GPCLK pcm_clk = {
    .clk_periph = PCMCLK,
    // .source = GPCLK_SRC_PLLD,
    .source = GPCLK_SRC_OSC,
    .speed = 8000 * 32 * 2
};

I2S i2s = {
    .frame_size = 32,
    .rx_data_size = 24,
    .tx_data_size = 24,
    .bclk = bclk,// Sclk, 18
    .fs = fs, // WS on the mic, 19
    .din = din, // SD on mic, 20
    .dout = dout,
    .clk = &pcm_clk 
    // L/R mic -> GND
};

CC1101 radio = {
    .spi_device = SPI_DEVICE_1, // CSn on GPIO7 (CE1)
    .gdo0 = {5},
    .gdo2 = {23},
};

// second module for loopback testing, shares SCLK/MOSI/MISO with radio
CC1101 radio_b = {
    .spi_device = SPI_DEVICE_0, // CSn on GPIO8 (CE0)
    .gdo0 = {6},
    .gdo2 = {22},
};


Pin josh = {26};