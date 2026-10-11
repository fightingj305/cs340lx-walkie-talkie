#pragma once
// mono i2s master driver for this project; we don't have to worry about stereo thank god
#include "gpio.h"
#include "gpclk.h"

#define PCM_BASE 0x20203000

enum {
    PCM_CS_A       = PCM_BASE + 0x00,
    PCM_FIFO_A     = PCM_BASE + 0x04,
    PCM_MODE_A     = PCM_BASE + 0x08,
    PCM_RXC_A      = PCM_BASE + 0x0C,
    PCM_TXC_A      = PCM_BASE + 0x10,
    PCM_DREQ_A     = PCM_BASE + 0x14,
    PCM_INTEN_A    = PCM_BASE + 0x18,
    PCM_INTSTC_A  = PCM_BASE + 0x1C,
    PCM_GRAY       = PCM_BASE + 0x20,
};

enum PCM_CS_Bits {
    PCM_CS_EN                 = 1 << 0,
    PCM_CS_RXON               = 1 << 1,
    PCM_CS_TXON               = 1 << 2,
    PCM_CS_TXCLR              = 1 << 3,
    PCM_CS_RXCLR              = 1 << 4,

    PCM_CS_TXTHR_MASK         = 0x3 << 5,
    PCM_CS_TXTHR_EMPTY        = 0x0 << 5,
    PCM_CS_TXTHR_ONE          = 0x1 << 5,
    PCM_CS_TXTHR_QUARTER      = 0x2 << 5,
    PCM_CS_TXTHR_HALF         = 0x3 << 5,

    PCM_CS_RXTHR_MASK         = 0x3 << 7,
    PCM_CS_RXTHR_ONE          = 0x0 << 7,
    PCM_CS_RXTHR_QUARTER      = 0x1 << 7,
    PCM_CS_RXTHR_HALF         = 0x2 << 7,
    PCM_CS_RXTHR_FULL         = 0x3 << 7,

    PCM_CS_DMAEN              = 1 << 9,

    PCM_CS_TXSYNC             = 1 << 13,
    PCM_CS_RXSYNC             = 1 << 14,

    PCM_CS_TXERR              = 1 << 15,
    PCM_CS_RXERR              = 1 << 16,

    PCM_CS_TXW                = 1 << 17,
    PCM_CS_RXR                = 1 << 18,

    PCM_CS_TXD                = 1 << 19,
    PCM_CS_RXD                = 1 << 20,

    PCM_CS_TXE                = 1 << 21,
    PCM_CS_RXF                = 1 << 22,

    PCM_CS_RXSEX              = 1 << 23,
    PCM_CS_SYNC               = 1 << 24
};

enum PCM_MODE_Bits {
    PCM_MODE_FSLEN_BIT      = 0,
    PCM_MODE_FLEN_BIT       = 10,

    PCM_MODE_FSI             = 1 << 20,
    PCM_MODE_FSM             = 1 << 21,

    PCM_MODE_CLKI            = 1 << 22,
    PCM_MODE_CLKM            = 1 << 23,

    PCM_MODE_FTXP            = 1 << 24,
    PCM_MODE_FRXP            = 1 << 25,

    PCM_MODE_PDME            = 1 << 26,
    PCM_MODE_PDMN            = 1 << 27,

    PCM_MODE_CLK_DIS         = 1 << 28
};

// same bits for txc/rxc
enum PCM_CHCFG_Bits {
    PCM_CH1_EN               = 1 << 30,
    PCM_CH1_WID_MASK         = 0xF << 16,
    PCM_CH1_POS_MASK         = 0x3FF << 20,

    PCM_CH1_WEX              = 1 << 31,
    PCM_CH1_WID_BIT          = 16,

    PCM_CH1_POS_0            = 0 << 20,
    PCM_CH1_POS_1            = 1 << 20,
    PCM_CH1_POS_8            = 8 << 20,
    PCM_CH1_POS_16           = 16 << 20,
    PCM_CH1_POS_24           = 24 << 20,
    PCM_CH1_POS_32           = 32 << 20,
    PCM_CH1_POS_33           = 33 << 20,

    PCM_CH2_EN               = 1 << 14,
    PCM_CH2_WID_MASK         = 0xF << 0,
    PCM_CH2_POS_MASK         = 0x3FF << 4,

    PCM_CH2_WEX              = 1 << 15,
    PCM_CH2_WID_BIT          = 0,

    PCM_CH2_POS_0            = 0 << 4,
    PCM_CH2_POS_1            = 1 << 4,
    PCM_CH2_POS_8            = 8 << 4,
    PCM_CH2_POS_16           = 16 << 4,
    PCM_CH2_POS_24           = 24 << 4,
    PCM_CH2_POS_32           = 32 << 4,
    PCM_CH2_POS_33           = 33 << 4,
};

enum PCM_INT_Bits {
    PCM_INT_TXW            = 1 << 0,
    PCM_INT_RXR            = 1 << 1,
    PCM_INT_TXERR          = 1 << 2,
    PCM_INT_RXERR          = 1 << 3,
};

typedef struct I2S_t {
    uint32_t frame_size;
    uint32_t rx_data_size;
    uint32_t tx_data_size;

    Pin bclk;
    Pin fs;
    Pin din;
    Pin dout;

    GPCLK *clk;
} I2S;

void PCM_Clock_Setup(GPCLK *clk);
void I2S_Init(I2S *i2s);
void I2S_Wait_2(void);
void I2S_Clear_FIFO(void);
void I2S_Clear_Flags(void);
void I2S_Enable_Comms(void);
void I2S_Disable_Comms(void);
void I2S_Disable(void);
bool I2S_TX_Full(void);
bool I2S_RX_Ready(void);
void I2S_Enable_IRQ(void);
void I2S_Clear_IRQ(void);
void I2S_Send_Value(uint32_t frame); // sends a left word with empty right word
uint32_t I2S_Read_Value(); // will read a whole frame and only give left word