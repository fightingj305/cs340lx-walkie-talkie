#include "miniuart.h"
#include <stdint.h>
#include "interrupts.h"
#include "hardware.h"
#include "utils.h"


int main() {
    interrupts_off();
    TIM_Disable();

    UART_Disable();
    UART_Config(&uart);
    UART_Enable();
    UART_Send_String("Booted!\n");


    I2S_Init(&i2s);
    
    I2S_Enable_Comms();
    I2S_Clear_Flags(); 

    CC1101_RX_Status status;
    while (1) {
        uint8_t mic_val[3]; 
        CC1101_Receive(&radio_b, mic_val, 3, &status, 1000000);
        uint32_t output = mic_val[0] << 16 | mic_val[1] << 8 | mic_val[0];
        I2S_Send_Value(output & 0x00FFFFFF);
    }
    
    return 0;
}