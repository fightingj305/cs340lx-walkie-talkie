#include "timer.h"
#include "spi.h"
#include <stdlib.h>
#include "miniuart.h"
#include "utils.h"
#include "hardware.h"


void SPI_Config(SPI *spi) {
    GPIO_Config(&spi->sclk, AF0);
    GPIO_Config(&spi->mosi, AF0);
    GPIO_Config(&spi->miso, AF0);
    GPIO_Config(&spi->cs0, AF0);
    GPIO_Config(&spi->cs1, AF0);
    
    SPI_Clear_FIFOs();

    ASSERT(spi->speed > 0);
    PUT32(SPI0_CLK, CORE_CLOCK_RATE / spi->speed);

    PUT32(SPI0_CS, (spi->cpha << SPI0_CS_CPHA_BIT) | (spi->cpol << SPI0_CS_CPOL_BIT));
}

void SPI_Clear_FIFOs(void) {
    PUT32(SPI0_CS, GET32(SPI0_CS) | (1 << SPI0_CS_CLEAR_RX_BIT) | (1 << SPI0_CS_CLEAR_TX_BIT));
}

bool SPI_TX_Full(void)
{
    return ~(GET32(SPI0_CS) >> SPI0_CS_TXD_BIT) & 1;
}

bool SPI_RX_Empty(void)
{
    return ~(GET32(SPI0_CS) >> SPI0_CS_RXD_BIT) & 1;
}

bool SPI_Transfer_Done(void)
{
    return (GET32(SPI0_CS) >> SPI0_CS_DONE_BIT) & 1;
}

uint8_t SPI_Transaction(SPI_Device spi_device, uint8_t byte) {
    uint32_t cs = GET32(SPI0_CS);
    cs = (cs & ~SPI0_CS_CS_MASK) | spi_device;
    PUT32(SPI0_CS, cs);

    SPI_Clear_FIFOs();
    
    PUT32(SPI0_CS, cs | (1 << SPI0_CS_TA_BIT));
    while (SPI_TX_Full());
    PUT32(SPI0_FIFO, byte);
    while (!SPI_Transfer_Done());
    while (SPI_RX_Empty());
    uint8_t output = (uint8_t)GET32(SPI0_FIFO);
    PUT32(SPI0_CS, cs);
    return output;
}


void SPI_Send_Multi(SPI_Device spi_device, uint32_t num_bytes, uint8_t *data) {
    uint32_t cs = GET32(SPI0_CS);
    cs = (cs & ~SPI0_CS_CS_MASK) | spi_device;
    PUT32(SPI0_CS, cs);

    SPI_Clear_FIFOs();
    
    PUT32(SPI0_CS, cs | (1 << SPI0_CS_TA_BIT));
    
    uint32_t sent = 0;
    while (sent < num_bytes || !SPI_Transfer_Done())
    {
        while (!SPI_TX_Full() && sent < num_bytes)
        {
            PUT32(SPI0_FIFO, data[sent++]);
        }
        while (!SPI_RX_Empty())
        {
            (void)GET32(SPI0_FIFO);
        }
    }
    PUT32(SPI0_CS, cs);
}

void SPI_Transaction_Multi(SPI_Device spi_device, uint32_t num_bytes, uint8_t *writes, uint8_t *data) {
    uint32_t cs = GET32(SPI0_CS);
    cs = (cs & ~SPI0_CS_CS_MASK) | spi_device;
    PUT32(SPI0_CS, cs);

    SPI_Clear_FIFOs();
    
    PUT32(SPI0_CS, cs | (1 << SPI0_CS_TA_BIT));
    
    uint32_t sent = 0;
    uint32_t received = 0;
    while (sent < num_bytes || received < num_bytes || !SPI_Transfer_Done())
    {
        while (!SPI_TX_Full() && sent < num_bytes)
        {
            PUT32(SPI0_FIFO, writes[sent++]);
        }
        while (!SPI_RX_Empty() && received < num_bytes)
        {
            data[received++] = (uint8_t)GET32(SPI0_FIFO);
        }
    }
    PUT32(SPI0_CS, cs);
}
