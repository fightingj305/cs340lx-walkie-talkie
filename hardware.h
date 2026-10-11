#pragma once

#include "gpio.h"
#include "miniuart.h"
#include "i2c.h"
#include "i2s.h"
#include "gpclk.h"
#include "spi.h"

// all hardware defines here!
extern UART uart;
extern const Pin rx;
extern const Pin tx;

extern const SPI spi;
extern const Pin mosi;
extern const Pin miso;
extern const Pin sclk;
extern const Pin cs0;
extern const Pin cs1;

extern I2S i2s;
extern GPCLK pcm_clk;
extern const Pin bclk;
extern const Pin fs;
extern const Pin din;
extern const Pin dout;