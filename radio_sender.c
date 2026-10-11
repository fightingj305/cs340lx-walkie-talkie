#include "miniuart.h"
#include <stdint.h>
#include "interrupts.h"
#include "hardware.h"
#include "utils.h"

volatile uint32_t i2s_frame = 0;

void __attribute__((interrupt("IRQ"))) interrupt_vector(void) {
    if (IRQ_Check_Interrupt(IRQ_PCM_INT)) {
        i2s_frame = I2S_Read_Value();
        I2S_Clear_IRQ();
    } else if(TIM_Pending()) {
        uint8_t data[3] = {i2s_frame >> 16, (i2s_frame >> 8) & 0xFF, i2s_frame & 0xFF};
        CC1101_Send_Packet(&radio, data ,3);
    }
}



int main() {
    interrupts_off();
    // TIM_Disable();
    TIM_Enable();
    TIM_Enable_IRQ();
    TIM_Set_Freq(48000);

    UART_Disable();
    UART_Config(&uart);
    UART_Enable();
    UART_Send_String("Booted!\n");


    I2S_Init(&i2s);
    
    I2S_Enable_Comms();
    I2S_Clear_Flags(); 
    I2S_Enable_IRQ();

    SPI_Config(&spi);
    CC1101_Init(&radio, &CC1101_PRESET_250K);

    interrupts_on();
    while (1) {
        UART_Print_Hex(i2s_frame);
    }
    
    return 0;
}