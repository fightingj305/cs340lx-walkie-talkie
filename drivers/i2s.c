#include "i2s.h"
#include "gpclk.h"
#include "utils.h"
#include "interrupts.h"

void PCM_Clock_Setup(GPCLK *clk) {
    GPCLK_Init(clk);
}

void I2S_Init(I2S *i2s) {
    ASSERT(i2s->rx_data_size <= 32 && i2s->tx_data_size <= 32);
    ASSERT((i2s->frame_size & 0x3FF) == i2s->frame_size);
    PCM_Clock_Setup(i2s->clk);
    PUT32(PCM_CS_A, PCM_CS_EN);
    PUT32(PCM_MODE_A, PCM_MODE_CLK_DIS); // disable clock prior to configuring mode?
    PUT32(PCM_MODE_A, PCM_MODE_FSI | PCM_MODE_CLKI | (((i2s->frame_size * 2)-1) << PCM_MODE_FLEN_BIT) | (i2s->frame_size << PCM_MODE_FSLEN_BIT));
    uint32_t rx_data_size = ((i2s->rx_data_size - 8) << PCM_CH1_WID_BIT);
    uint32_t tx_data_size = ((i2s->tx_data_size - 8) << PCM_CH1_WID_BIT);
    if (i2s->rx_data_size >= 24) {
        rx_data_size = PCM_CH1_WEX | (i2s->rx_data_size - 24) << PCM_CH1_WID_BIT;
    }
    if (i2s->tx_data_size >= 24) {
        tx_data_size = PCM_CH1_WEX | (i2s->tx_data_size - 24) << PCM_CH1_WID_BIT;
    }
    PUT32(PCM_RXC_A, 
        PCM_CH1_EN | PCM_CH1_POS_1 | rx_data_size |
        PCM_CH2_EN | PCM_CH2_POS_33 | (rx_data_size >> 16)
    );

    PUT32(PCM_TXC_A, 
        PCM_CH1_EN | PCM_CH1_POS_1 | tx_data_size |
        PCM_CH2_EN | PCM_CH2_POS_33 | (tx_data_size >> 16)
    );

    I2S_Clear_Flags();
    I2S_Clear_FIFO();

    if (i2s->bclk.pin_num == 18 && i2s->fs.pin_num == 19 && i2s->din.pin_num == 20 && i2s->dout.pin_num == 21) {
        GPIO_Config(&i2s->bclk, AF0);
        GPIO_Config(&i2s->fs, AF0);
        GPIO_Config(&i2s->din, AF0);
        GPIO_Config(&i2s->dout, AF0);
    }
    else if (i2s->bclk.pin_num == 28 && i2s->fs.pin_num == 29 && i2s->din.pin_num == 30 && i2s->dout.pin_num == 31) {
        GPIO_Config(&i2s->bclk, AF2);
        GPIO_Config(&i2s->fs, AF2);
        GPIO_Config(&i2s->din, AF2);
        GPIO_Config(&i2s->dout, AF2);
    }
    else {
        PANIC("Invalid I2S GPIO CONFIG");
    }
}

void I2S_Wait_2(void) {
    if (GET32(PCM_MODE_A) & PCM_MODE_CLK_DIS)
        return;
    uint32_t current_value = GET32(PCM_CS_A);
    PUT32(PCM_CS_A, current_value ^ PCM_CS_SYNC);
    while ((GET32(PCM_CS_A) & PCM_CS_SYNC) != ((current_value ^ PCM_CS_SYNC) & PCM_CS_SYNC));
}

void I2S_Clear_Flags(void) {
    RMW_OR(PCM_CS_A, PCM_CS_RXSYNC | PCM_CS_TXSYNC | PCM_CS_RXERR | PCM_CS_TXERR);
}

void I2S_Clear_FIFO(void) {
    RMW_OR(PCM_CS_A, PCM_CS_RXCLR | PCM_CS_TXCLR);
    I2S_Wait_2();
}

void I2S_Enable_Comms(void) {
    RMW_OR(PCM_CS_A, PCM_CS_RXON | PCM_CS_TXON);
}
void I2S_Disable_Comms(void) {
    RMW_AND(PCM_CS_A, ~(PCM_CS_RXON | PCM_CS_TXON));
}

bool I2S_TX_Full(void) {
    return (GET32(PCM_CS_A) & PCM_CS_TXD) == 0;
}

bool I2S_RX_Ready(void) {
    return (GET32(PCM_CS_A) & PCM_CS_RXD) != 0;
}

void I2S_Enable_IRQ(void) {
    RMW_AND(PCM_CS_A, ~PCM_CS_RXTHR_MASK);
    RMW_OR(PCM_CS_A, PCM_CS_RXTHR_ONE); // set rxr threshold to one sample
    PUT32(PCM_INTEN_A, PCM_INT_RXR);
    IRQ_Enable_Interrupt(IRQ_PCM_INT);
}

void I2S_Clear_IRQ(void) {
    RMW_AND(PCM_INTSTC_A, PCM_INT_RXR);
}

void I2S_Send_Value(uint32_t frame) {
    while (I2S_TX_Full());
    PUT32(PCM_FIFO_A, frame);
    while (I2S_TX_Full());
    PUT32(PCM_FIFO_A, 0);
}

uint32_t I2S_Read_Value(void) {
    uint32_t out;
    while (!I2S_RX_Ready());
    out = GET32(PCM_FIFO_A);
    while (!I2S_RX_Ready());
    GET32(PCM_FIFO_A);
    return out;
}
