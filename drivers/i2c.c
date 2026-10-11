#include "i2c.h"
#include "timer.h"
#include "utils.h"

void I2C_Enable(I2C *i2c) {
    RMW_OR(i2c->bsc + I2C_C, (1 << I2C_ENABLE_BIT));
}

void I2C_Disable(I2C *i2c) {
    RMW_AND(i2c->bsc + I2C_C, ~(1 << I2C_ENABLE_BIT));
}

void I2C_Init(I2C *i2c) {
    ASSERT(i2c->slave_addr == (i2c->slave_addr & 0x7F));

    PUT32(i2c->bsc + I2C_DIV, i2c->speed_hz == 0 ? 0 : CORE_CLOCK_RATE / i2c->speed_hz);
    PUT32(i2c->bsc + I2C_A, i2c->slave_addr);
    if (i2c->bsc == BSC0) {
        if ((i2c->sda.pin_num == 0 && i2c->scl.pin_num == 1) || (i2c->sda.pin_num == 28 && i2c->scl.pin_num == 29)) {
            GPIO_Config(&i2c->sda, AF0);
            GPIO_Config(&i2c->scl, AF0);
        }
        else if (i2c->sda.pin_num == 44 && i2c->scl.pin_num == 45) {
            GPIO_Config(&i2c->sda, AF1);
            GPIO_Config(&i2c->scl, AF1);
        }
        else {
            PANIC("Invalid I2C pins!");
        }
    }
    else if (i2c->bsc == BSC1) {
        if (i2c->sda.pin_num == 2 && i2c->scl.pin_num == 3) {
            GPIO_Config(&i2c->sda, AF0);
            GPIO_Config(&i2c->scl, AF0);
        }
        else if (i2c->sda.pin_num == 44 && i2c->scl.pin_num == 45) {
            GPIO_Config(&i2c->sda, AF2);
            GPIO_Config(&i2c->scl, AF2);
        }
        else {
            PANIC("Invalid I2C pins!");
        }
    }
    else {
        PANIC("BSC2 does not have I2C lines!");
    }
}

void I2C_Start_Transaction(I2C *i2c) {
    RMW_OR(i2c->bsc + I2C_C, (1 << I2C_ST_BIT));
}

bool I2C_Transaction_Done(I2C *i2c) {
    uint32_t status = GET32(i2c->bsc + I2C_S);
    bool done = (status >> I2C_DONE_BIT) & 1;
    if (done) {
        PUT32(i2c->bsc + I2C_S, 1 << I2C_DONE_BIT); // clear the done bit
    }
    return done;
}

void I2C_Clear_Flags(I2C *i2c) {
    PUT32(i2c->bsc + I2C_S, (1 << I2C_DONE_BIT) |
                            (1 << I2C_ERR_BIT) |
                            (1 << I2C_CLKT_BIT));
}

void I2C_Clear_FIFO(I2C *i2c) {
    RMW_OR(i2c->bsc + I2C_C, (1 << I2C_CLEAR_BIT));
    RMW_AND(i2c->bsc + I2C_C, ~(1 << I2C_CLEAR_BIT));
}

bool I2C_TX_FIFO_Full(I2C *i2c) {
    return ((GET32(i2c->bsc + I2C_S) >> I2C_TXD_BIT) & 1) == 0;
}

bool I2C_TX_FIFO_Empty(I2C *i2c) {
    return (GET32(i2c->bsc + I2C_S) >> I2C_TXE_BIT) & 1;
}

bool I2C_RX_FIFO_Ready(I2C *i2c) {
    return (GET32(i2c->bsc + I2C_S) >> I2C_RXD_BIT) & 1;
}

bool I2C_Check_Error(I2C *i2c) {
    uint32_t status = GET32(i2c->bsc + I2C_S);

    if (status & (1 << I2C_ERR_BIT)) {
        PUT32(i2c->bsc + I2C_S, 1 << I2C_ERR_BIT);
        return true;
    }

    if (status & (1 << I2C_CLKT_BIT)) {
        PUT32(i2c->bsc + I2C_S, 1 << I2C_CLKT_BIT);
        return true;
    }

    return false;
}

void I2C_Send_Data(I2C *i2c, uint32_t num_bytes, uint8_t *data) {
    ASSERT((num_bytes & 0xFFFF) == num_bytes);

    I2C_Clear_Flags(i2c);
    RMW_AND(i2c->bsc + I2C_C, ~(1 << I2C_READ_BIT));
    PUT32(i2c->bsc + I2C_DLEN, num_bytes & 0xFFFF);

    I2C_Clear_FIFO(i2c);
    uint32_t bytes_sent = 0;
   
    while (bytes_sent < num_bytes && !I2C_TX_FIFO_Full(i2c)) {
        PUT32(i2c->bsc + I2C_FIFO, data[bytes_sent]);
        bytes_sent++;
    }

    I2C_Start_Transaction(i2c);
    while (bytes_sent < num_bytes) {
        while (I2C_TX_FIFO_Full(i2c));
        PUT32(i2c->bsc + I2C_FIFO, data[bytes_sent]);
        bytes_sent++;
    }
    while (!I2C_Transaction_Done(i2c)) {
        if (I2C_Check_Error(i2c)) return;
    }
}

void I2C_Receive_Data(I2C *i2c, uint32_t num_bytes, uint8_t *data) {
    ASSERT((num_bytes & 0xFFFF) == num_bytes);

    I2C_Clear_Flags(i2c);
    RMW_OR(i2c->bsc + I2C_C, (1 << I2C_READ_BIT));
    PUT32(i2c->bsc + I2C_DLEN, num_bytes & 0xFFFF);

    I2C_Clear_FIFO(i2c);
    uint32_t bytes_received= 0;
   
    I2C_Start_Transaction(i2c);
    while (bytes_received < num_bytes) {
        while (!I2C_RX_FIFO_Ready(i2c));
        data[bytes_received] = GET32(i2c->bsc + I2C_FIFO);
        bytes_received++;
    }
    while (!I2C_Transaction_Done(i2c)) {
        if (I2C_Check_Error(i2c)) return;
    }
}