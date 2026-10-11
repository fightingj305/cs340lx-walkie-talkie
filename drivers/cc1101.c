#include "cc1101.h"
#include "spi.h"
#include "timer.h"
#include "utils.h"
#include "miniuart.h"

// base frequency word: f_carrier = 26 MHz / 2^16 * FREQ (datasheet section 21)
// 433.92 MHz -> 0x10B071. for 915 MHz use 0x23313B, for 868.3 MHz use 0x216656
#define CC1101_FREQ_WORD 0x10B071

#define CC1101_TX_TIMEOUT_US 100000

#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

// 9.6 kBaud 2-FSK. slow enough for rtl_433 -X 'n=cc1101,m=FSK_PCM,s=104,l=104,...' to decode.
// FSCTRL1 / MDMCFG2 / AGCCTRL2 are just the reset defaults, listed so switching presets is clean
static const CC1101_Reg_Setting settings_9k6_sdr[] = {
    {CC1101_MDMCFG4,  0x88}, // RX BW 203 kHz, DRATE_E = 8
    {CC1101_MDMCFG3,  0x83}, // DRATE_M = 131 -> 9.596 kBaud (104 us / bit)
    {CC1101_MDMCFG2,  0x02}, // (reset default) DC filter on, 2-FSK, no manchester, 16/16 sync word
    {CC1101_DEVIATN,  0x35}, // +-20.6 kHz deviation
    {CC1101_FSCTRL1,  0x0F}, // (reset default) IF = 381 kHz
    {CC1101_AGCCTRL2, 0x03}, // (reset default) all gains allowed, MAGN_TARGET = 33 dB
};

const CC1101_Preset CC1101_PRESET_9K6_SDR = {
    .name = "9.6 kBaud 2-FSK (SDR)",
    .settings = settings_9k6_sdr,
    .num_settings = ARRAY_LEN(settings_9k6_sdr),
};

// 250 kBaud GFSK: the setting characterized in datasheet table 7 (127 kHz deviation, 540 kHz
// filter, 304 kHz IF). about 217 kbit/s of payload after packet overhead -- room for raw 8 kHz
// 8-bit audio. too fast for rtl_433 to decode, but still visible on a waterfall.
// SmartRF Studio's preset also tunes FOCCFG/BSCFG/AGCCTRL1/0/FREND1/FSCAL*/TEST0 -- add those if
// this turns out flaky
static const CC1101_Reg_Setting settings_250k[] = {
    {CC1101_MDMCFG4,  0x2D}, // RX BW 541 kHz (CHANBW_E = 0, M = 2), DRATE_E = 13
    {CC1101_MDMCFG3,  0x3B}, // DRATE_M = 59 -> 249.94 kBaud (4 us / bit)
    {CC1101_MDMCFG2,  0x12}, // DC filter on, GFSK, no manchester, 16/16 sync word
    {CC1101_DEVIATN,  0x62}, // +-126.9 kHz deviation
    {CC1101_FSCTRL1,  0x0C}, // IF = 304.7 kHz
    {CC1101_AGCCTRL2, 0x07}, // all gains allowed, MAGN_TARGET = 42 dB (what section 17.4 uses at 250k)
};

const CC1101_Preset CC1101_PRESET_250K = {
    .name = "250 kBaud GFSK",
    .settings = settings_250k,
    .num_settings = ARRAY_LEN(settings_250k),
};

uint8_t CC1101_Strobe(CC1101 *cc1101, uint8_t command) {
    return SPI_Transaction(cc1101->spi_device, command);
}

void CC1101_Set_Reg(CC1101 *cc1101, uint8_t address, uint8_t value) {
    uint8_t data[2] = {(address & 0x3F) | CC1101_WRITE_SINGLE, value};
    SPI_Send_Multi(cc1101->spi_device, 2, data);
}

uint8_t CC1101_Read_Reg(CC1101 *cc1101, uint8_t address) {
    uint8_t tx[2] = {(address & 0x3F) | CC1101_READ_SINGLE, 0x00};
    uint8_t rx[2] = {0, 0};
    SPI_Transaction_Multi(cc1101->spi_device, 2, tx, rx);
    return rx[1];
}

// 0x30-0x3D are strobes without the burst bit, status registers with it
uint8_t CC1101_Read_Status_Reg(CC1101 *cc1101, uint8_t address) {
    uint8_t tx[2] = {(address & 0x3F) | CC1101_READ_BURST, 0x00};
    uint8_t rx[2] = {0, 0};
    SPI_Transaction_Multi(cc1101->spi_device, 2, tx, rx);
    return rx[1];
}

void CC1101_Write_Burst(CC1101 *cc1101, uint8_t address, const uint8_t *values, uint32_t len) {
    uint8_t buf[CC1101_FIFO_SIZE + 1];
    ASSERT(len <= CC1101_FIFO_SIZE);
    buf[0] = (address & 0x3F) | CC1101_WRITE_BURST;
    for (uint32_t i = 0; i < len; i++) {
        buf[i + 1] = values[i];
    }
    SPI_Send_Multi(cc1101->spi_device, len + 1, buf);
}

void CC1101_Reset(CC1101 *cc1101) {
    // we can't watch SO between CSn low and the first clock edge with the hardware CS,
    // so just give the crystal / reset plenty of time instead (datasheet 19.1)
    TIM_SYS_Delay_Millis(1);
    CC1101_Strobe(cc1101, CC1101_SRES);
    TIM_SYS_Delay_Millis(1);
}

void CC1101_Apply_Preset(CC1101 *cc1101, const CC1101_Preset *preset) {
    // frequency / modem registers should only change in IDLE (section 21)
    CC1101_Strobe(cc1101, CC1101_SIDLE);
    for (uint32_t i = 0; i < preset->num_settings; i++) {
        CC1101_Set_Reg(cc1101, preset->settings[i].reg, preset->settings[i].value);
    }
}

// previous version:
// void CC1101_Init(CC1101 *cc1101) {
// why: takes the preset to apply after the shared settings below
void CC1101_Init(CC1101 *cc1101, const CC1101_Preset *preset) {
    GPIO_Config(&cc1101->gdo0, INPUT);
    GPIO_Config(&cc1101->gdo2, INPUT);

    CC1101_Reset(cc1101);

    // previous version: data rate / deviation left at reset defaults
    // // everything not set here stays at reset defaults: 2-FSK, ~115 kBaud, +-47 kHz deviation,
    // // 4 byte preamble, 0xD391 sync word, variable length packets (section 29)
    // CC1101_Set_Reg(cc1101, CC1101_MDMCFG4, 0x8C);  // (reset default) RX BW 203 kHz, DRATE_E = 12
    // CC1101_Set_Reg(cc1101, CC1101_MDMCFG3, 0x22);  // (reset default) DRATE_M = 34 -> 115.051 kBaud
    // CC1101_Set_Reg(cc1101, CC1101_DEVIATN, 0x47);  // (reset default) +-47.6 kHz deviation
    //
    // why the change: at 115 kBaud the NESDR + rtl_433 couldn't decode the packets. rtl_433 -A saw
    // one 1.2 ms burst per second (matches 136 bits @ 115 kBaud, so TX itself works) but couldn't split
    // it into bits -- at -s 1024k that's only ~9 samples/bit and rtl_433 is tuned for slower remotes.
    // 9.6 kBaud gives ~107 samples/bit, so rtl_433 should be able to demod it. lower rate also means
    // better RX sensitivity (table 7), which a walkie-talkie link wants anyway. deviation scaled down
    // with the data rate so the signal stays narrow (~50 kHz occupied vs ~200 kHz before).

    // previous version: 9.6 kBaud modem settings written directly here
    // // everything not set here stays at reset defaults: 2-FSK, 203 kHz RX filter,
    // // 4 byte preamble, 0xD391 sync word, variable length packets (section 29)
    // CC1101_Set_Reg(cc1101, CC1101_MDMCFG4, 0x88);  // RX BW 203 kHz, DRATE_E = 8
    // CC1101_Set_Reg(cc1101, CC1101_MDMCFG3, 0x83);  // DRATE_M = 131 -> 9.596 kBaud (104 us / bit)
    // CC1101_Set_Reg(cc1101, CC1101_DEVIATN, 0x35);  // +-20.6 kHz deviation
    //
    // why the change: these now live in CC1101_PRESET_9K6_SDR (same values) so we can pick
    // between that and CC1101_PRESET_250K. everything below is shared by all presets.

    // everything not set here or in the preset stays at reset defaults:
    // 4 byte preamble, 0xD391 sync word, variable length packets (section 29)
    CC1101_Set_Reg(cc1101, CC1101_IOCFG0, 0x06);   // GDO0: high after sync word sent, low at end of packet
    CC1101_Set_Reg(cc1101, CC1101_PKTCTRL0, 0x05); // whitening off, FIFO mode, CRC on, variable length
    CC1101_Set_Reg(cc1101, CC1101_MCSM0, 0x18);    // calibrate on IDLE->RX/TX, PO_TIMEOUT = 2
    CC1101_Set_Reg(cc1101, CC1101_MCSM1, 0x30);    // CCA default, go to IDLE after RX and after TX
    CC1101_Set_Reg(cc1101, CC1101_FREQ2, (CC1101_FREQ_WORD >> 16) & 0xFF);
    CC1101_Set_Reg(cc1101, CC1101_FREQ1, (CC1101_FREQ_WORD >> 8) & 0xFF);
    CC1101_Set_Reg(cc1101, CC1101_FREQ0, CC1101_FREQ_WORD & 0xFF);
    CC1101_Set_Reg(cc1101, CC1101_FREND0, 0x10);   // PA_POWER = 0 -> always use PATABLE[0], no ramping
    // added for RX: in variable length mode PKTLEN is the max length accepted (reset default 255).
    // capping it at what fits in the FIFO makes the chip drop packets with a corrupted length byte
    CC1101_Set_Reg(cc1101, CC1101_PKTLEN, CC1101_MAX_PAYLOAD);

    // ~+10 dBm at 433/915 (table 39). single byte write only touches index 0
    CC1101_Set_Reg(cc1101, CC1101_PATABLE, 0xC0);

    CC1101_Apply_Preset(cc1101, preset);
}

bool CC1101_Send_Packet(CC1101 *cc1101, const uint8_t *data, uint8_t len) {
    uint8_t buf[CC1101_MAX_PAYLOAD + 1];
    ASSERT(len > 0 && len <= CC1101_MAX_PAYLOAD);

    CC1101_Strobe(cc1101, CC1101_SIDLE);
    CC1101_Strobe(cc1101, CC1101_SFTX);

    // variable length mode: first FIFO byte is the payload length
    buf[0] = len;
    for (uint8_t i = 0; i < len; i++) {
        buf[i + 1] = data[i];
    }
    CC1101_Write_Burst(cc1101, CC1101_FIFO, buf, len + 1);

    CC1101_Strobe(cc1101, CC1101_STX);

    // wait on MARCSTATE rather than GDO0 so a GDO0 wiring problem doesn't look like a radio problem
    bool saw_gdo0 = false;
    bool left_idle = false;
    uint32_t start = TIM_SYS_Get_Ticks();
    while (1) {
        if (GPIO_Read(&cc1101->gdo0) == HIGH) {
            saw_gdo0 = true;
        }
        uint8_t state = CC1101_Read_Status_Reg(cc1101, CC1101_MARCSTATE) & 0x1F;
        if (state != CC1101_MARC_IDLE) {
            left_idle = true;
        } else if (left_idle) {
            break;
        }
        if (state == CC1101_MARC_TXFIFO_UNDERFLOW) {
            UART_Send_String("cc1101: TX FIFO underflow\n");
            CC1101_Strobe(cc1101, CC1101_SFTX);
            return false;
        }
        if (TIM_SYS_Get_Ticks() - start > CC1101_TX_TIMEOUT_US) {
            UART_Send_String("cc1101: TX timeout, MARCSTATE = ");
            UART_Print_Hex(state);
            CC1101_Strobe(cc1101, CC1101_SIDLE);
            return false;
        }
    }

    if (!saw_gdo0) {
        UART_Send_String("cc1101: warning, GDO0 never went high\n");
    }
    return true;
}

void CC1101_Start_RX(CC1101 *cc1101) {
    CC1101_Strobe(cc1101, CC1101_SIDLE);
    CC1101_Strobe(cc1101, CC1101_SFRX);
    CC1101_Strobe(cc1101, CC1101_SRX);
}

// MCSM1.RXOFF_MODE = IDLE, so the radio drops out of RX once a full packet is in the FIFO.
// polls MARCSTATE rather than GDO0 so it also catches the overflow state
bool CC1101_Wait_RX_Done(CC1101 *cc1101, uint32_t timeout_us) {
    uint32_t start = TIM_SYS_Get_Ticks();
    while (TIM_SYS_Get_Ticks() - start < timeout_us) {
        uint8_t state = CC1101_Read_Status_Reg(cc1101, CC1101_MARCSTATE) & 0x1F;
        if (state == CC1101_MARC_IDLE || state == CC1101_MARC_RXFIFO_OVERFLOW) {
            return true;
        }
    }
    return false;
}

// status registers can read corrupt if they change mid-read (section 10.3), so read until stable
static uint8_t CC1101_Read_RXBYTES(CC1101 *cc1101) {
    uint8_t prev = CC1101_Read_Status_Reg(cc1101, CC1101_RXBYTES);
    while (1) {
        uint8_t cur = CC1101_Read_Status_Reg(cc1101, CC1101_RXBYTES);
        if (cur == prev) {
            return cur;
        }
        prev = cur;
    }
}

int CC1101_Read_Packet(CC1101 *cc1101, uint8_t *buf, uint8_t max_len, CC1101_RX_Status *status) {
    uint8_t rxbytes = CC1101_Read_RXBYTES(cc1101);
    uint8_t count = rxbytes & 0x7F;

    if (rxbytes & 0x80) {
        UART_Send_String("cc1101: RX FIFO overflow\n");
        CC1101_Strobe(cc1101, CC1101_SIDLE);
        CC1101_Strobe(cc1101, CC1101_SFRX);
        return -1;
    }
    if (count == 0) {
        return -1;
    }

    // FIFO holds: length byte, payload, RSSI, LQI | CRC_OK. read it all in one burst
    // no = {0} initializers: gcc turns them into memset, which we don't have with -nostdlib
    uint8_t tx[CC1101_FIFO_SIZE + 1];
    uint8_t rx[CC1101_FIFO_SIZE + 1];
    ASSERT(count <= CC1101_FIFO_SIZE);
    tx[0] = CC1101_FIFO | CC1101_READ_BURST;
    for (uint8_t i = 1; i <= count; i++) {
        tx[i] = 0;
    }
    SPI_Transaction_Multi(cc1101->spi_device, count + 1, tx, rx);
    uint8_t *fifo = &rx[1];

    uint8_t len = fifo[0];
    if (len + 3 != count || len > max_len) {
        UART_Send_String("cc1101: bad RX length byte ");
        UART_Send_Int(len);
        UART_Send_String(" with RXBYTES = ");
        UART_Print_Int(count);
        CC1101_Strobe(cc1101, CC1101_SFRX);
        return -1;
    }

    for (uint8_t i = 0; i < len; i++) {
        buf[i] = fifo[1 + i];
    }

    // RSSI is 2's complement in half dB steps, offset 74 dB (section 17.3)
    status->rssi_dbm = (int32_t)(int8_t)fifo[1 + len] / 2 - 74;
    status->lqi = fifo[2 + len] & 0x7F;
    status->crc_ok = (fifo[2 + len] >> 7) & 1;
    return len;
}

int CC1101_Receive(CC1101 *cc1101, uint8_t *buf, uint8_t max_len, CC1101_RX_Status *status, uint32_t timeout_us) {
    CC1101_Start_RX(cc1101);

    // right after SRX the chip can still read IDLE for a moment, which Wait_RX_Done would take
    // as "packet done". wait until it has actually left IDLE first
    uint32_t start = TIM_SYS_Get_Ticks();
    while ((CC1101_Read_Status_Reg(cc1101, CC1101_MARCSTATE) & 0x1F) == CC1101_MARC_IDLE) {
        if (TIM_SYS_Get_Ticks() - start > timeout_us) {
            return -1;
        }
    }

    if (!CC1101_Wait_RX_Done(cc1101, timeout_us)) {
        CC1101_Strobe(cc1101, CC1101_SIDLE);
        return -1;
    }
    return CC1101_Read_Packet(cc1101, buf, max_len, status);
}
