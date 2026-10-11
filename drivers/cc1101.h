#pragma once
// minimal cc1101 driver: register access + fixed config + single-packet TX (FIFO / packet mode)

#include <stdint.h>
#include <stdbool.h>
#include "gpio.h"
#include "spi.h"

typedef struct CC1101_t {
    SPI_Device spi_device; // which hardware CE line CSn is wired to
    Pin gdo0;              // configured as "sync sent / end of packet" (IOCFG0 = 0x06)
    Pin gdo2;              // left at reset default: CHIP_RDYn (low when crystal is running)
} CC1101;

// configuration registers (datasheet table 43)
enum CC1101_Registers {
    CC1101_IOCFG2   = 0x00,
    CC1101_IOCFG1   = 0x01,
    CC1101_IOCFG0   = 0x02,
    CC1101_FIFOTHR  = 0x03,
    CC1101_SYNC1    = 0x04,
    CC1101_SYNC0    = 0x05,
    CC1101_PKTLEN   = 0x06,
    CC1101_PKTCTRL1 = 0x07,
    CC1101_PKTCTRL0 = 0x08,
    CC1101_CHANNR   = 0x0A,
    CC1101_FSCTRL1  = 0x0B,
    CC1101_FREQ2    = 0x0D,
    CC1101_FREQ1    = 0x0E,
    CC1101_FREQ0    = 0x0F,
    CC1101_MDMCFG4  = 0x10,
    CC1101_MDMCFG3  = 0x11,
    CC1101_MDMCFG2  = 0x12,
    CC1101_MDMCFG1  = 0x13,
    CC1101_MDMCFG0  = 0x14,
    CC1101_DEVIATN  = 0x15,
    CC1101_MCSM1    = 0x17,
    CC1101_MCSM0    = 0x18,
    CC1101_AGCCTRL2 = 0x1B,
    CC1101_FREND0   = 0x22,
    CC1101_FSCAL1   = 0x25,
    CC1101_PATABLE  = 0x3E,
    CC1101_FIFO     = 0x3F,
};

// status registers (table 44), must be read with the burst bit set
enum CC1101_Status_Registers {
    CC1101_PARTNUM   = 0x30,
    CC1101_VERSION   = 0x31,
    CC1101_RSSI      = 0x34,
    CC1101_MARCSTATE = 0x35,
    CC1101_PKTSTATUS = 0x38,
    CC1101_TXBYTES   = 0x3A,
    CC1101_RXBYTES   = 0x3B,
};

// command strobes (table 42)
enum CC1101_Commands {
    CC1101_SRES    = 0x30,
    CC1101_SFSTXON = 0x31,
    CC1101_SCAL    = 0x33,
    CC1101_SRX     = 0x34,
    CC1101_STX     = 0x35,
    CC1101_SIDLE   = 0x36,
    CC1101_SFRX    = 0x3A,
    CC1101_SFTX    = 0x3B,
    CC1101_SNOP    = 0x3D,
};

// MARCSTATE values (page 93)
enum CC1101_Marc_States {
    CC1101_MARC_IDLE              = 0x01,
    CC1101_MARC_RX                = 0x0D,
    CC1101_MARC_RXFIFO_OVERFLOW   = 0x11,
    CC1101_MARC_TX                = 0x13,
    CC1101_MARC_TXFIFO_UNDERFLOW  = 0x16,
};

#define CC1101_WRITE_SINGLE 0x00
#define CC1101_WRITE_BURST  0x40
#define CC1101_READ_SINGLE  0x80
#define CC1101_READ_BURST   0xC0

#define CC1101_FIFO_SIZE 64
#define CC1101_MAX_PAYLOAD 61 // 64 byte fifo minus length byte, leave some slack

uint8_t CC1101_Strobe(CC1101 *cc1101, uint8_t command); // returns chip status byte
void CC1101_Set_Reg(CC1101 *cc1101, uint8_t address, uint8_t value);
uint8_t CC1101_Read_Reg(CC1101 *cc1101, uint8_t address);
uint8_t CC1101_Read_Status_Reg(CC1101 *cc1101, uint8_t address);
void CC1101_Write_Burst(CC1101 *cc1101, uint8_t address, const uint8_t *values, uint32_t len);

// a preset is the modem side of the config (data rate, filter, deviation, modulation, IF, AGC).
// every preset writes the same set of registers so you can switch presets without a reset.
// both ends of a link must use the same preset
typedef struct CC1101_Reg_Setting_t {
    uint8_t reg;
    uint8_t value;
} CC1101_Reg_Setting;

typedef struct CC1101_Preset_t {
    char *name;
    const CC1101_Reg_Setting *settings;
    uint32_t num_settings;
} CC1101_Preset;

extern const CC1101_Preset CC1101_PRESET_9K6_SDR; // 9.6 kBaud 2-FSK, decodable by rtl_433 on the NESDR
extern const CC1101_Preset CC1101_PRESET_250K;    // 250 kBaud GFSK, datasheet table 7 setting, for audio

void CC1101_Reset(CC1101 *cc1101);
// previous version:
// void CC1101_Init(CC1101 *cc1101); // assumes SPI_Config already done
// why: modem settings moved into presets so we can switch between SDR-debuggable and fast configs
void CC1101_Init(CC1101 *cc1101, const CC1101_Preset *preset); // assumes SPI_Config already done
void CC1101_Apply_Preset(CC1101 *cc1101, const CC1101_Preset *preset); // radio must be IDLE
bool CC1101_Send_Packet(CC1101 *cc1101, const uint8_t *data, uint8_t len); // blocks until sent

// the two status bytes the chip appends after each received packet (PKTCTRL1.APPEND_STATUS, table 27/28)
typedef struct CC1101_RX_Status_t {
    int32_t rssi_dbm;
    uint8_t lqi;   // lower is better
    bool crc_ok;
} CC1101_RX_Status;

void CC1101_Start_RX(CC1101 *cc1101); // returns immediately, chip listens on its own
bool CC1101_Wait_RX_Done(CC1101 *cc1101, uint32_t timeout_us); // true once the radio has left RX (packet in FIFO)
int CC1101_Read_Packet(CC1101 *cc1101, uint8_t *buf, uint8_t max_len, CC1101_RX_Status *status); // payload len, or -1

// convenience: Start_RX + Wait_RX_Done + Read_Packet. blocks up to timeout_us for one packet.
// returns payload len, or -1 on timeout / bad packet. check status->crc_ok before trusting the data
int CC1101_Receive(CC1101 *cc1101, uint8_t *buf, uint8_t max_len, CC1101_RX_Status *status, uint32_t timeout_us);
