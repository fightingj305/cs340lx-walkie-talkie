#include "miniuart.h"
#include <stdint.h>
#include "interrupts.h"
#include "hardware.h"
#include "utils.h"
#include "rng.h"

int main() {
    interrupts_off();

    UART_Disable();
    UART_Config(&uart);
    UART_Enable();
    UART_Send_String("Booted!\n");

    RNG_Init();

    while (1) {
        UART_Send_String("Number ");
        UART_Print_Int(RNG_Randint(100, 400));
        TIM_SYS_Delay_Millis(1000);
    }
    
    return 0;
}