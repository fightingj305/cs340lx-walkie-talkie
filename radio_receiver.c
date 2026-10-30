#include "miniuart.h"
#include <stdint.h>
#include "interrupts.h"
#include "hardware.h"
#include "utils.h"
#include "dma.h"

DMA_SECTION DMA_CB dma_buf[1024];
DMA_Write writes[1024];

#define BUFFER_SIZE 4096
#define BUFFER_COUNT 8
volatile uint32_t ring_buffer[BUFFER_COUNT][BUFFER_SIZE];
uint32_t head = 0;
uint32_t current_buffer = BUFFER_COUNT - 1;

uint32_t output;

void __attribute__((interrupt("IRQ"))) interrupt_vector(void) {
    // if (TIM_Pending()) {
    //     // I2S_Send_Value(output & 0x00FFFFFF);
    //     // uint32_t value = (head % (BUFFER_SIZE / 32))  < (BUFFER_SIZE / 64) ? 0x7FFFFF : 0 ;
    //     uint32_t value = (head % (BUFFER_SIZE / 32))  * 0x7FFFFF/(BUFFER_SIZE / 32);
    //     // UART_Print_Hex(value);
    //     ring_buffer[current_buffer][head] = value;
    //     head++;
    //     if (head >= BUFFER_SIZE) {
    //         head = 0;
    //         uint32_t next_buffer = (current_buffer + 1) % BUFFER_COUNT;
    //         while (DMA_Get_Current_Source(DMA_Channel_4) >= (uint32_t)&ring_buffer[next_buffer][0] &&
    //             DMA_Get_Current_Source(DMA_Channel_4) < (uint32_t)&ring_buffer[next_buffer][BUFFER_SIZE]);
    //         current_buffer = next_buffer;
    //     }
    //     I2S_Send_Value(value);
    //     TIM_Clear_Pending();
    // }
    // else 
    if (GPIO_Check_Any_Pin_IRQ() && GPIO_Check_Pin_IRQ(&radio_b.gdo2)) {
        CC1101_RX_Status status;
        uint8_t mic_val[60];
        int retval = CC1101_Read_Packet(&radio_b, mic_val, 60, &status);
        for (uint32_t i = 0; i < 60; i++) {
            // uint32_t value = mic_val[i] << 16 |  mic_val[i + 1] << 8 |  mic_val[i + 2];
            // uint32_t value = mic_val[i] << 16;
            uint32_t value = (head % BUFFER_SIZE / 16)  < BUFFER_SIZE / 32 ? 0x7FFFFF : 0 ;
            UART_Print_Hex(value);
            ring_buffer[current_buffer][head] = value;
            head++;
            if (head >= BUFFER_SIZE) {
                head = 0;
                uint32_t next_buffer = (current_buffer + 1) % BUFFER_COUNT;
                while (DMA_Get_Current_Source(DMA_Channel_4) >= (uint32_t)&ring_buffer[next_buffer][0] &&
                    DMA_Get_Current_Source(DMA_Channel_4) < (uint32_t)&ring_buffer[next_buffer][BUFFER_SIZE]);
                current_buffer = next_buffer;
            }
            I2S_Send_Value(value);
        }
        GPIO_Clear_IRQ(&radio_b.gdo2);
        CC1101_Start_RX(&radio_b);
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
    CC1101_Init_IRQ(&radio_b);
    CC1101_Start_RX(&radio_b);

    I2S_Init(&i2s);
    
    I2S_Enable_Comms();
    I2S_Clear_Flags(); 

    I2S_Setup_DMA();

    TIM_Enable();
    TIM_Set_Frequency(10000);
    TIM_Enable_IRQ();

    interrupts_on();

    for (uint32_t i = 0; i < BUFFER_COUNT; i++) {
        writes[i] = (DMA_Write) {
                .src = (uint32_t)&ring_buffer[i][0],
                .dst = PCM_FIFO_A,
                .size = 4 * BUFFER_SIZE,
                .periph = DMA_DREQ_PCM_TX,
                .periph_read = false,
                .inc_src = true,
                .inc_dst = false
        };
    }


    // DMA_Transmit(DMA_Channel_4, writes, dma_buf, BUFFER_COUNT);
    // DMA_Start(DMA_Channel_4);

    while (1) {
        UART_Send_Int(head);
        UART_Send_String(" ");
        UART_Print_Hex(ring_buffer[current_buffer][head]);
    }
    
    return 0;
}