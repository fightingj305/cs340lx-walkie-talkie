#pragma once

#define DMA_BASE 0x20007000

#define DMA_SECTION __attribute__((section(".dma_mem"))) __attribute__((aligned(32)))

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    DMA_Channel_0 = DMA_BASE,
    DMA_Channel_1 = DMA_BASE + 0x100,
    DMA_Channel_2 = DMA_BASE + 0x200,
    DMA_Channel_3 = DMA_BASE + 0x300,
    DMA_Channel_4 = DMA_BASE + 0x400,
    DMA_Channel_5 = DMA_BASE + 0x500,
    DMA_Channel_6 = DMA_BASE + 0x600,
    DMA_Channel_7 = DMA_BASE + 0x700,
    DMA_Channel_8 = DMA_BASE + 0x800,
    DMA_Channel_9 = DMA_BASE + 0x900,
    DMA_Channel_10 = DMA_BASE + 0xA00,
    DMA_Channel_11 = DMA_BASE + 0xB00,
    DMA_Channel_12 = DMA_BASE + 0xC00,
    DMA_Channel_13 = DMA_BASE + 0xD00,
    DMA_Channel_14 = DMA_BASE + 0xE00,
} DMA_Channel;

enum {
    DMA_CS = 0x0,
    DMA_CONBLK_AD = 0x4,
    DMA_TI = 0x8,
    DMA_SOURCE_AD = 0xC,
    DMA_DEST_AD = 0x10,
    DMA_TXFR_LEN = 0x14,
    DMA_STRIDE = 0x18,
    DMA_NEXTCONBLK = 0x1c,
    DMA_DEBUG = 0x20
};

enum {
    DMA_CS_ACTIVE = 1 << 0,
    DMA_CS_END = 1 << 1,
    DMA_CS_INT = 1 << 2,
    DMA_CS_RESET = 1 << 31,

    DMA_TI_INTEN = 1 << 0,
    DMA_TDMODE = 1 << 1,
    DMA_WAIT_RESP = 1 << 3,
    DMA_DEST_INC = 1 << 4,
    DMA_DEST_WIDTH_128 = 1 << 5,
    DMA_DEST_DREQ = 1 << 6,
    DMA_DEST_IGNORE = 1 << 7,
    DMA_SRC_INC = 1 << 8,
    DMA_SRC_WIDTH_128 = 1 << 9,
    DMA_SRC_DREQ = 1 << 10,
    DMA_SRC_IGNORE = 1 << 11,
    DMA_BURST_LENGTH_1 = 1 << 12,
    DMA_BURST_LENGTH_2 = 2 << 12,
    DMA_BURST_LENGTH_4 = 4 << 12,
    DMA_BURST_LENGTH_8 = 8 << 12,
    DMA_BURST_LENGTH_BIT = 12,
    DMA_PERMAP_BIT = 16,
    DMA_WAITS_BIT = 21,
    DMA_NO_WIDE_BURSTS = 1 << 26,
};

typedef enum {
    DMA_DREQ_NONE,
    DMA_DREQ_DSI,
    DMA_DREQ_PCM_TX,
    DMA_DREQ_PCM_RX,
    DMA_DREQ_SMI,
    DMA_DREQ_PWM,
    DMA_DREQ_SPI_TX,
    DMA_DREQ_SPI_RX,
    DMA_DREQ_BSC_TX,
    DMA_DREQ_BSC_RX,
    DMA_DREQ_UNUSED,
    DMA_DREQ_EMMC,
    DMA_DREQ_UART_TX,
    DMA_DREQ_SD_HOST,
    DMA_DREQ_UART_RX,
    DMA_DREQ_DSI_ALT,
    DMA_DREQ_MCTX,
    DMA_DREQ_HDMI,
    DMA_DREQ_MCRX,
} DMA_DREQ_PERMAP;

typedef struct {
    uint32_t ti;
    uint32_t source_ad;
    uint32_t dest_ad;
    uint32_t txfr_len;
    uint32_t stride;
    uint32_t nextconbk;
    uint32_t reserved[2];
} __attribute__((aligned(32))) DMA_CB;

typedef struct {
    volatile uint32_t src;
    volatile uint32_t dst;
    uint32_t size;
    
    DMA_DREQ_PERMAP periph;
    bool periph_read;
    bool inc_src;
    bool inc_dst;
} DMA_Write;

void DMA_Reset(DMA_Channel channel);
void DMA_Start(DMA_Channel channel);
void DMA_Pause(DMA_Channel channel);
void DMA_Set_Block(DMA_Channel channel, uint32_t cb);
void DMA_Transmit(DMA_Channel channel, DMA_Write *writes, DMA_CB *dma_buf, uint32_t num_writes);
void DMA_Transmit_Terminating(DMA_Channel channel, DMA_Write *writes, DMA_CB *dma_buf, uint32_t num_writes);
void DMA_CB_Print(DMA_CB *dma_cb);
uint32_t DMA_Get_Current_Source(DMA_Channel channel);
uint32_t DMA_Get_Current_Destination(DMA_Channel channel);