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
    uint32_t count = 0;
    while (1) {
        UART_Send_String("Alive ");
        UART_Print_Int(count);
        count++;
        TIM_SYS_Delay_Millis(1000);
    }
    
    return 0;
}