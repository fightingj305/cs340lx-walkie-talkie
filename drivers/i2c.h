#pragma once

#include "gpio.h"

typedef enum BSC_t {
    BSC0 = 0x20205000,
    BSC1 = 0x20804000,
    BSC2 = 0x20805000 // used in HDMI interface, avoid
} BSC;

// SDA0/SCL0: pin 0/1 AF0 or pin 28/29 AF0 or 44/45 AF1
// SDA1/SCL1: pin 2/3 AF0 or 44/45 AF2

// address enums
enum {
    I2C_C = 0x0,
    I2C_S = 0x4,
    I2C_DLEN = 0x8,
    I2C_A = 0xC,
    I2C_FIFO = 0x10,
    I2C_DIV = 0x14,
    I2C_DEL = 0x18,
    I2C_CLKT = 0x1C,
};

// bit enums
enum {
    I2C_READ_BIT = 0,
    I2C_CLEAR_BIT = 4,
    I2C_ST_BIT = 7,
    I2C_INTD_BIT = 8,
    I2C_INTT_BIT = 9,
    I2C_INTR_BIT = 10,
    I2C_ENABLE_BIT = 15,

    I2C_TA_BIT = 0,
    I2C_DONE_BIT = 1,
    I2C_TXW_BIT = 2,
    I2C_RXR_BIT = 3,
    I2C_TXD_BIT = 4,
    I2C_RXD_BIT = 5,
    I2C_TXE_BIT = 6,
    I2C_RXF_BIT = 7,
    I2C_ERR_BIT = 8,
    I2C_CLKT_BIT = 9,
};

typedef struct I2C_t { // should define a separate I2C for each slave
    BSC bsc;
    Pin sda;
    Pin scl;
    uint32_t speed_hz; // default 0 will set 100kHz
    uint32_t slave_addr;
} I2C;

void I2C_Enable(I2C *i2c);
void I2C_Disable(I2C *i2c);
void I2C_Init(I2C *i2c);
void I2C_Start_Transaction(I2C *i2c);
bool I2C_Transaction_Done(I2C *i2c);
void I2C_Clear_Flags(I2C *i2c);
void I2C_Clear_FIFO(I2C *i2c);
bool I2C_TX_FIFO_Full(I2C *i2c);
bool I2C_TX_FIFO_Empty(I2C *i2c);
bool I2C_RX_FIFO_Ready(I2C *i2c);
bool I2C_Check_Error(I2C *i2c);
void I2C_Send_Data(I2C *i2c, uint32_t num_bytes, uint8_t *data);
void I2C_Receive_Data(I2C *i2c, uint32_t num_bytes, uint8_t *data);