#include "miniuart.h"
#include <stdint.h>
#include "interrupts.h"
#include "hardware.h"
#include "utils.h"

uint32_t output;

void __attribute__((interrupt("IRQ"))) interrupt_vector(void) {
    if (TIM_Pending()) {
        I2S_Send_Value(output & 0x00FFFFFF);
        TIM_Clear_Pending();
    }
}

int main() {
    interrupts_off();
    TIM_Disable();

    UART_Disable();
    UART_Config(&uart);
    UART_Enable();
    UART_Send_String("Booted!\n");

    SPI_Config(&spi);
    CC1101_Init(&radio_b, &CC1101_PRESET_250K);

    I2S_Init(&i2s);
    
    I2S_Enable_Comms();
    I2S_Clear_Flags(); 

    TIM_Enable();
    TIM_Set_Frequency(48000);
    TIM_Enable_IRQ();

    interrupts_on();

    CC1101_RX_Status status;
    while (1) {
        uint8_t mic_val[3] = {0, 0, 0};
        CC1101_Receive(&radio_b, mic_val, 3, &status, 10000);
        if (status.crc_ok) {
            output = mic_val[0] << 16 | mic_val[1] << 8 | mic_val[2];
        }
    }
    
    return 0;
}