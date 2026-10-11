#pragma once

#include "gpio.h"

#define GPCLK_BASE        (0x20101000)
#define GPCLK_PASSWD  (0x5A << 24)

#define PLLA_Freq 700000000
#define PLLC_Freq 250000000
#define PLLD_Freq 500000000
#define OSC_Freq 19200000

typedef enum CM_Reg_t {
    GPCLK_GP0CTL = GPCLK_BASE + 0x70,
    GPCLK_GP0DIV = GPCLK_BASE + 0x74,
    GPCLK_GP1CTL = GPCLK_BASE + 0x78,
    GPCLK_GP1DIV = GPCLK_BASE + 0x7C,
    GPCLK_GP2CTL = GPCLK_BASE + 0x80,
    GPCLK_GP2DIV = GPCLK_BASE + 0x84,
    GPCLK_PCMCTL = GPCLK_BASE + 0x98,
    GPCLK_PCMDIV = GPCLK_BASE + 0x9C,
    GPCLK_PWMCTL = GPCLK_BASE + 0xA0,
    GPCLK_PWMDIV = GPCLK_BASE + 0xA4
} CM_Reg;

typedef enum {
    GPCLK_SRC_GND  = 0,
    GPCLK_SRC_OSC  = 1,
    GPCLK_SRC_TESTDEBUG0 = 2,
    GPCLK_SRC_TESTDEBUG1 = 3,
    GPCLK_SRC_PLLA = 4,
    GPCLK_SRC_PLLC = 5,
    GPCLK_SRC_PLLD = 6,
    GPCLK_SRC_HDMI = 7
} GPCLK_ClkSrc; // must set this while busy is false and not while setting enable

enum {
    GPCLK_CTL_ENAB = (1 << 4),
    GPCLK_CTL_KILL = (1 << 5), // datasheet suggests against this
    GPCLK_CTL_BUSY = (1 << 7),

    GPCLK_CTL_FLIP = (1 << 8),

    GPCLK_CTL_MASH_0 = (0 << 9),
    GPCLK_CTL_MASH_1 = (1 << 9),
    GPCLK_CTL_MASH_2 = (2 << 9),
    GPCLK_CTL_MASH_3 = (3 << 9)
};

enum {
    GPCLK_DIV_DIVF_SHIFT = 0,
    GPCLK_DIV_DIVI_SHIFT = 12
};

typedef enum CM_Periph_t {
    GPCLK0,
    GPCLK1,
    GPCLK2,
    PCMCLK,
    PWMCLK
} CM_Periph;

typedef struct GPCLK_t { // define GPCLK for every different clock
    uint32_t speed; // in Hz
    CM_Periph clk_periph;
    GPCLK_ClkSrc source;
    Pin output_pin;
} GPCLK;

CM_Reg GPCLK_Get_Ctl(GPCLK *clk); // get control register for given peripheral
CM_Reg GPCLK_Get_Div(GPCLK *clk); // get divider register for given peripheral
void GPCLK_Enable(GPCLK *clk);
void GPCLK_Disable(GPCLK *clk);
void GPCLK_Init(GPCLK *clk);
void GPCLK_Pin_Init(GPCLK *clk); // Configures corresponding GPIO pin for GPCLK output; the one instance where a pin in the struct is driver-defined