#pragma once

#include "gpio.h"
#include "miniuart.h"
#include "i2c.h"
#include "i2s.h"
#include "gpclk.h"
#include "spi.h"
#include "cc1101.h"

// all hardware defines here!
extern UART uart;
extern const Pin rx;
extern const Pin tx;

// previous version:
// extern const SPI spi;
// why: const dropped so &spi can be passed to SPI_Config (see hardware.c)
extern SPI spi;
extern const Pin mosi;
extern const Pin miso;
extern const Pin sclk;
extern const Pin cs0;
extern const Pin cs1;

extern CC1101 radio;
extern CC1101 radio_b;

extern I2S i2s;
extern GPCLK pcm_clk;
extern const Pin bclk;
extern const Pin fs;
extern const Pin din;
extern const Pin dout;

extern Pin josh;