#include "miniuart.h"
#include "utils.h"

void UART_Enable(void) {
    uint32_t old_state = GET32(AUX_ENABLES);
    PUT32(AUX_ENABLES, old_state | 1);
    
}

void UART_Disable(void) {
    uint32_t old_state = GET32(AUX_ENABLES);
    PUT32(AUX_ENABLES, old_state & (~1));
}

void UART_Config(UART *uart) {
    
    GPIO_Config(&uart->tx_pin, AF5);
    GPIO_Config(&uart->rx_pin, AF5);
    const uint32_t reg_value = (CORE_CLOCK_RATE / (8 * uart->baud_rate)) - 1;
    
    PUT32(AUX_MU_IER_REG, 0); // not necessary but i'll humor them
    PUT32(AUX_MU_IIR_REG, 0b11 << 1); // clear both FIFOs
    PUT32(AUX_MU_LCR_REG, uart->num_bits == UART_8_BIT ? 0b11 : 0b00);
    PUT32(AUX_MU_BAUD_REG, reg_value);
    PUT32(AUX_MU_CNTL_REG, 0b11);
    
}

void UART_Send_Byte(const uint8_t byte) {
    while (!UART_TX_Ready());
    PUT32(AUX_MU_IO_REG, byte);
}

void UART_Send_String(char *str) {
    for (char *c = str; *c != '\0'; c++) {
        UART_Send_Byte(*c);
    }
}

void UART_Send_Int(uint32_t num) {
    if (num == 0) {
        UART_Send_Byte('0');
    }
    char output[11] = "\0\0\0\0\0\0\0\0\0\0";
    uint8_t i = 0;
    while (num > 0 && i < sizeof(output)) {
        output[i] = '0' + num % 10;
        num = num / 10;
        i++;
    }
    
    for (char *c = output + i - 1; c >= output; c--) {
        if (*c) {
            UART_Send_Byte(*c);
        }
    }
}

void UART_Print_Int(uint32_t num) {
    UART_Send_Int(num);
    UART_Send_Byte('\n');
}


void UART_Send_Hex(uint32_t num) {
    UART_Send_String("0x");
    for (int8_t i = 28; i >= 0; i-= 4) {
        uint8_t number = num >> i & 0xF;
        if (number < 10) {
            UART_Send_Byte('0' + number);
        }
        else {
            UART_Send_Byte('A' + number - 10);
        }
    }
}


void UART_Print_Hex(uint32_t num) {
    UART_Send_Hex(num);
    UART_Send_Byte('\n');
}
uint8_t UART_Receive_Byte(void) {
    while(!UART_RX_Ready());
    return (uint8_t) (GET32(AUX_MU_IO_REG) & 0xFF);
}

void UART_Flush_TX(void) {
    while (!UART_TX_Empty());
}

bool UART_TX_Empty(void) {
    return ((GET32(AUX_MU_LSR_REG) >> 6) & 1) == 1;
}

bool UART_TX_Ready(void) {
    return ((GET32(AUX_MU_LSR_REG) >> 5) & 1) == 1;
}

bool UART_RX_Ready(void) {
    return (GET32(AUX_MU_LSR_REG) & 1) == 1;
}
