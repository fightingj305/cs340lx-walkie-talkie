#include "miniuart.h"
#include <stdint.h>
#include "interrupts.h"
#include "hardware.h"
#include "utils.h"

void __attribute__((interrupt("IRQ"))) interrupt_vector(void) {
    if (IRQ_Check_Interrupt(IRQ_PCM_INT)) {
        uint32_t mic_val = I2S_Read_Value();
        I2S_Send_Value(mic_val & 0x00FFFFFF);
        I2S_Clear_IRQ();
    }
}

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
    I2S_Enable_IRQ();

    interrupts_on();
    uint32_t count = 0;
    while (1) {
        UART_Send_String("Alive ");
        UART_Print_Int(count);
        count++;
        TIM_SYS_Delay_Millis(1000);
    }
    
    return 0;
}