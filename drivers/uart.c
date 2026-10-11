#include "uart.h"
#include "timer.h"


void UART_Timer_Setup(UART *uart) {
    TIM_Disable();
    TIM_Set_Load(UART_DELAY);
    TIM_Clear_Pending();
    TIM_Enable();
    GPIO_Config(&uart->UART_TX_Pin, OUTPUT);
    GPIO_Set_High(&uart->UART_TX_Pin);
    uart->UART_TX_bit = UART_STOP_BIT + 1;
}

void UART_Receiver_Setup(UART_RX *uart) {
    TIM_Disable();
    TIM_Set_Load(UART_DELAY);
    TIM_Clear_Pending();
    TIM_Enable();
    GPIO_Config(&uart->UART_RX_Pin, INPUT);
    uart->UART_RX_bit = UART_STOP_BIT + 1;
    uart->num_received = 0;
}


void UART_SendByte(UART *uart, const uint8_t byte) {
    
    if (uart->UART_TX_bit <= UART_STOP_BIT) {
        return;
    }

    uart->UART_TX_byte = byte;
    uart->UART_TX_bit = UART_START_BIT;
}

void UART_TimerHandler(UART *uart) {
    if (uart->UART_TX_bit > UART_STOP_BIT) {
        return;
    }

    if (uart->UART_TX_bit == UART_START_BIT) {
        GPIO_Set(&uart->UART_TX_Pin, LOW);
    } else if (uart->UART_TX_bit == UART_STOP_BIT) {
        GPIO_Set(&uart->UART_TX_Pin, HIGH);
    } else {
        GPIO_Set(&uart->UART_TX_Pin, (Pin_Output) ((uart->UART_TX_byte >> (uart->UART_TX_bit - 1)) & 1));
    }

    uart->UART_TX_bit++;
}

void UART_RXTimerHandler(UART_RX *uart) {
    if (uart->UART_RX_bit > UART_STOP_BIT) {
        return;
    }

    if (uart->UART_RX_bit == UART_START_BIT) {
        TIM_Set_Load(UART_DELAY * 1.5);
    } else if (uart->UART_RX_bit == UART_START_BIT + 1) {
        TIM_Set_Load(UART_DELAY);
        uart->UART_RX_byte |= (GPIO_Read(&uart->UART_RX_Pin) << (uart->UART_RX_bit - 1));
    } else if (uart->UART_RX_bit == UART_STOP_BIT) {
        TIM_Disable();
        uart->num_received++;
    } else {
        uart->UART_RX_byte |= (GPIO_Read(&uart->UART_RX_Pin) << (uart->UART_RX_bit - 1));
    }

    uart->UART_RX_bit++;
}

void UART_SendBytes(UART *uart, const uint8_t *byte, uint32_t num_bytes) {
    for (uint32_t i = 0; i < num_bytes; i++) {
        UART_SendByte(uart, byte[i]);
        while (uart->UART_TX_bit <= UART_STOP_BIT);
    }
}