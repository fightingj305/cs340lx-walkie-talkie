#pragma once
// this is really just a hardcoded 8-bit spi1 driver. tough luck for any spi2 users out there

#include <stdint.h>
#include "gpio.h"
#include <stdbool.h>

// AUX shit

// #define SPI1_AUX_EN_BIT 1 
// #define SPI1_VAR_CS_BIT 15
// #define SPI1_VAR_WIDTH_BIT 14
// #define SPI1_CPOL_BIT 7
// #define SPI1_OUT_RISING_BIT 8
// #define SPI1_IN_RISING_BIT 10
// #define SPI1_EN_BIT 11
// #define SPI1_SHIFT_LEN 8
// #define SPI1_TX_FULL_BIT 4
// #define SPI1_TX_EMPTY_BIT 3
// #define SPI1_RX_EMPTY_BIT 2

// #define AUX_BASE 0x20215000
// enum {
//     AUX_SPI0_CNTL0 = AUX_BASE + 0x80,
//     AUX_SPI0_CNTL1 = AUX_BASE + 0x84,
//     AUX_SPI0_STAT = AUX_BASE + 0x88,
//     AUX_SPI0_IO = AUX_BASE + 0x8C,
//     AUX_SPI0_PEEK = AUX_BASE + 0x90
// };

// SPI0_CS Register Bits

#define SPI0_CS_LEN_LONG_BIT        25  // Enable Long data word in LoSSI mode
#define SPI0_CS_DMA_LEN_BIT         24  // Enable DMA mode in LoSSI
#define SPI0_CS_CSPOL2_BIT          23  // Chip Select 2 Polarity
#define SPI0_CS_CSPOL1_BIT          22  // Chip Select 1 Polarity
#define SPI0_CS_CSPOL0_BIT          21  // Chip Select 0 Polarity
#define SPI0_CS_RXF_BIT             20  // RX FIFO Full
#define SPI0_CS_RXR_BIT             19  // RX FIFO needs Reading (¾ full)
#define SPI0_CS_TXD_BIT             18  // TX FIFO can accept Data
#define SPI0_CS_RXD_BIT             17  // RX FIFO contains Data
#define SPI0_CS_DONE_BIT            16  // Transfer Done
#define SPI0_CS_LEN_BIT             13  // LoSSI enable
#define SPI0_CS_REN_BIT             12  // Read Enable (Bidirectional mode)
#define SPI0_CS_ADCS_BIT            11  // Automatically Deassert CS
#define SPI0_CS_INTR_BIT            10  // Interrupt on RXR
#define SPI0_CS_INTD_BIT            9   // Interrupt on DONE
#define SPI0_CS_DMAEN_BIT           8   // DMA Enable
#define SPI0_CS_TA_BIT              7   // Transfer Active
#define SPI0_CS_CSPOL_BIT           6   // Chip Select Polarity
#define SPI0_CS_CLEAR_RX_BIT        5   // Clear RX FIFO (write 1)
#define SPI0_CS_CLEAR_TX_BIT        4   // Clear TX FIFO (write 1)
#define SPI0_CS_CPOL_BIT            3   // Clock Polarity
#define SPI0_CS_CPHA_BIT            2   // Clock Phase
#define SPI0_CS_CS_BITS             0   // Chip Select (bits 1:0)

#define SPI0_CS_CS_MASK 0b11

#define SPI0_BASE 0x20204000
enum {
    SPI0_CS     = SPI0_BASE + 0x00,
    SPI0_FIFO   = SPI0_BASE + 0x04,
    SPI0_CLK    = SPI0_BASE + 0x08,
    SPI0_DLEN   = SPI0_BASE + 0x0C,
    SPI0_LTOH   = SPI0_BASE + 0x10,
    SPI0_DC     = SPI0_BASE + 0x14
};

typedef enum SPI_CPOL_t {
    CPOL_0 = 0, // clock idles at low
    CPOL_1, // clock idles at high
} SPI_CPOL;

typedef enum SPI_CPHA_t {
    CPHA_0 = 0, // data output when SCLK transitions to idle voltage, sample when SCLK transitions from idle voltage
    CPHA_1, // data output when SCLK transitions from idle voltage, sample when SCLK transitions to idle voltage
} SPI_CPHA;

typedef enum SPI_Edge_t {
    SPI_CLK_RISING,
    SPI_CLK_FALLING,
} SPI_Edge;

typedef enum SPI_Device_t { // each struct corresponds to one peripheral device
    SPI_DEVICE_0 = 0,
    SPI_DEVICE_1,
} SPI_Device;

typedef struct SPI_t { 
    uint32_t speed; // in hz
    SPI_CPOL cpol;
    SPI_CPHA cpha;
    bool msb_first;

    Pin mosi;
    Pin miso;
    Pin sclk;
    Pin cs0;
    Pin cs1;
} SPI;


void SPI_Config(SPI *spi);
void SPI_Clear_FIFOs(void);
bool SPI_TX_Full(void);
bool SPI_RX_Empty(void);
bool SPI_Transfer_Done(void);
uint8_t SPI_Transaction(SPI_Device spi_device, uint8_t byte); // spi is full duplex so we have to use this interface
void SPI_Transaction_Multi(SPI_Device spi_device, uint32_t num_bytes, uint8_t *writes, uint8_t *data);
void SPI_Send_Multi(SPI_Device spi_device, uint32_t num_bytes, uint8_t *data); // ignores read values
