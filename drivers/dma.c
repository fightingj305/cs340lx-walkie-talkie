#include "dma.h"
#include "utils.h"
#include "miniuart.h"


void DMA_Reset(DMA_Channel channel) {
    RMW_OR(channel + DMA_CS, DMA_CS_RESET);
}

void DMA_Start(DMA_Channel channel) {
    PUT32(channel + DMA_CS, DMA_CS_ACTIVE);
}

void DMA_Set_Block(DMA_Channel channel, uint32_t cb) {
    PUT32(channel + DMA_CONBLK_AD, cb);
}

void DMA_Clear_Flags(DMA_Channel channel) {
    PUT32(channel + DMA_CS, DMA_CS_END | DMA_CS_INT);
}

void DMA_Transmit(DMA_Channel channel, DMA_Write *writes, DMA_CB *dma_buf, uint32_t num_writes) {
    ASSERT(num_writes > 0);
    ASSERT(((uint32_t)dma_buf & 0x1F) == 0);
    for (uint32_t i = 0; i < num_writes; i++) {
        uint32_t ti =   (writes[i].periph == DMA_DREQ_NONE ? 0 : 
                            (writes[i].periph_read ? DMA_SRC_DREQ : DMA_DEST_DREQ)
                        ) | 
                        (writes[i].periph << DMA_PERMAP_BIT) |
                        (writes[i].inc_src ? DMA_SRC_INC : 0) | 
                        (writes[i].inc_dst ? DMA_DEST_INC : 0) |
                        (DMA_WAIT_RESP);
        uint32_t next_blk = (i >= num_writes - 1) ? (uint32_t)&dma_buf[0] : (uint32_t)&dma_buf[i+1];
        DMA_CB block = {
            .ti = ti,
            .source_ad = mem_to_bus((uint32_t)writes[i].src),
            .dest_ad   = mem_to_bus((uint32_t)writes[i].dst),
            .txfr_len = writes[i].size,
            .stride = 0, // no 2d for now
            .nextconbk = mem_to_bus(next_blk)
        };
        dma_buf[i] = block;
    }
    DMB();
    DMA_Reset(channel);
    while (GET32(channel + DMA_CS) & DMA_CS_RESET);
    DMA_Clear_Flags(channel);
    uint32_t bus_addr = mem_to_bus((uint32_t)dma_buf);
    DMA_Set_Block(channel, mem_to_bus((uint32_t)dma_buf));
}

void DMA_Transmit_Terminating(DMA_Channel channel, DMA_Write *writes, DMA_CB *dma_buf, uint32_t num_writes) {
    DMA_Transmit(channel, writes, dma_buf, num_writes);
    dma_buf[num_writes - 1].nextconbk = 0;
}

void DMA_CB_Print(DMA_CB *dma_cb) {
    UART_Send_String("DMA CB at ");
    UART_Print_Hex((uint32_t) dma_cb);
    UART_Send_String("TI: ");
    UART_Print_Hex((uint32_t) dma_cb->ti);
    UART_Send_String("SRC: ");
    UART_Print_Hex((uint32_t) dma_cb->source_ad);
    UART_Send_String("DST: ");
    UART_Print_Hex((uint32_t) dma_cb->dest_ad);
    UART_Send_String("TXFRLEN: ");
    UART_Print_Hex((uint32_t) dma_cb->txfr_len);
    UART_Send_String("STRIDE: ");
    UART_Print_Hex((uint32_t) dma_cb->stride);
    UART_Send_String("NEXTCONBLK: ");
    UART_Print_Hex((uint32_t) dma_cb->nextconbk);
}

uint32_t DMA_Get_Current_Source(DMA_Channel channel) {
    return ((DMA_CB *) GET32(channel + DMA_CONBLK_AD))->source_ad;
}