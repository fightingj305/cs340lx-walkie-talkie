#include "miniuart.h"
#include <stdint.h>
#include "interrupts.h"
#include "hardware.h"
#include "utils.h"
#include "timer.h"
#include "dma.h"

# define BYTES_PER_SAMPLE 2
#define SAMPLES_PER_PACKET 30

DMA_SECTION DMA_CB dma_buf[1024];
DMA_Write writes[1024];

#define BUFFER_SIZE 4096
#define BUFFER_COUNT 8

volatile uint32_t ring_buffer[BUFFER_COUNT][BUFFER_SIZE];
uint32_t head = 0;
uint32_t current_buffer = BUFFER_COUNT - 1;

uint32_t output;

// void __attribute__((interrupt("IRQ"))) interrupt_vector(void) {
//     if (IRQ_Check_Interrupt(IRQ_PCM_INT)) {
//         // GPIO_Toggle(&josh);
//         uint32_t raw = I2S_Read_Value();
//         I2S_Clear_IRQ();
//     }
// }

int main() {
    interrupts_off();
    TIM_Disable();

    GPIO_Config(&josh, OUTPUT);

    UART_Disable();
    UART_Config(&uart);
    UART_Enable();
    UART_Send_String("Booted!\n");

    I2S_Init(&i2s);
    I2S_Enable_Comms();
    I2S_Clear_Flags();

    I2S_Setup_DMA();
    // I2S_Enable_IRQ();

    SPI_Config(&spi);
    CC1101_Init(&radio, &CC1101_PRESET_250K);

    interrupts_on();

    for (uint32_t i = 0; i < BUFFER_COUNT; i++) {
        writes[i] = (DMA_Write) {
            .src = PCM_FIFO_A,
            .dst = (uint32_t)&ring_buffer[i][0],
            .size = 4 * BUFFER_SIZE,
            .periph = DMA_DREQ_PCM_RX,
            .periph_read = true,
            .inc_src = false,
            .inc_dst = true
        };
    }

    // writes[0] = (DMA_Write) {
    //     .src = PCM_FIFO_A,
    //     .dst = (uint32_t)&ring_buffer[0][0],
    //     .size = 4 * BUFFER_SIZE,
    //     .periph = DMA_DREQ_PCM_RX,
    //     .periph_read = true,
    //     .inc_src = false,
    //     .inc_dst = true
    // };


    DMA_Transmit(DMA_Channel_4, writes, dma_buf, BUFFER_COUNT);
    // DMA_Transmit_Terminating(DMA_Channel_4, writes, dma_buf, 1);
    // uint32_t t0 = GET32(TIM_SYS_CLO);
    DMA_Start(DMA_Channel_4);

    // Let's read a word at a time first
    while(1) {
        // UART_Print_Hex(ring_buffer[current_buffer][0]);
        // UART_Print_Hex(ring_buffer[current_buffer][1]);
        // UART_Print_Hex(ring_buffer[current_buffer][2]);
        // UART_Print_Hex(ring_buffer[current_buffer][3]);

        // UART_Print_Hex(ring_buffer[current_buffer][BUFFER_SIZE - 4]);
        // UART_Print_Hex(ring_buffer[current_buffer][BUFFER_SIZE - 3]);
        // UART_Print_Hex(ring_buffer[current_buffer][BUFFER_SIZE - 2]);
        // UART_Print_Hex(ring_buffer[current_buffer][BUFFER_SIZE - 1]);


        // // head++;
        // head += BUFFER_SIZE;

        // Assemble 60 bit packet from wherever we are
        // uint8_t packet[SAMPLES_PER_PACKET * BYTES_PER_SAMPLE];

        // for(uint32_t packet_i = 0; packet_i < SAMPLES_PER_PACKET * BYTES_PER_SAMPLE; packet_i+= BYTES_PER_SAMPLE) {
        //     // Skip every other word
        //     for(uint32_t sample_word_i = 0; sample_word_i < 2; sample_word_i++) {
        //         if(sample_word_i == 0) {
        //             uint32_t word = ring_buffer[current_buffer][head];
        //             // packet[packet_i] = (word >> 16) & 0xFF;
        //             // packet[packet_i + 1] = (word >> 8) & 0xFF;
        //             // packet[packet_i] = 0xde;
        //             // packet[packet_i + 1] = 0xad;
        //         }

        //         head++;

        //         // Continue i next buffer if necessary
        //         if (head >= BUFFER_SIZE) {
        //             head = 0;
        //             uint32_t next_buffer = (current_buffer + 1) % BUFFER_COUNT;
        //             while (DMA_Get_Current_Destination(DMA_Channel_4) >= mem_to_bus((uint32_t) &ring_buffer[next_buffer][0]) &&
        //                 DMA_Get_Current_Destination(DMA_Channel_4) < mem_to_bus((uint32_t) &ring_buffer[next_buffer][BUFFER_SIZE]));
                    
        //             current_buffer = next_buffer;
        //         }
        //     }
        // }

        uint8_t buf[60];
        for(int i = 0; i < 60; i++) {
            buf[i] = 0xde;
        }
        CC1101_Send_Packet(&radio, buf, SAMPLES_PER_PACKET * BYTES_PER_SAMPLE);
        GPIO_Toggle(&josh);
    }

    // while (!(GET32(DMA_Channel_4 + DMA_CS) & DMA_CS_END));         // wait for the capture to finish

    // UART_Print_Hex(GET32(TIM_SYS_CLO) - t0);   // µs; expect ~256000 (0x3E800)
    // UART_Print_Hex(GET32(PCM_CS_A));           // RXERR=bit16, RXD=bit20, DMAEN=bit9
    // UART_Print_Hex(dma_buf[0].ti);   


    // UART_Print_Hex(GET32(DMA_Channel_4 + DMA_CS));        // bit 1 END, bit 8 ERROR
    // UART_Print_Hex(GET32(DMA_Channel_4 + DMA_DEST_AD));   // should end at mem_to_bus(&ring_buffer[0][BUFFER_SIZE])
    // UART_Print_Hex(GET32(DMA_Channel_4 + DMA_TXFR_LEN));  // bytes still to go; 0 when finished
    // UART_Print_Hex(GET32(DMA_Channel_4 + DMA_DEBUG));     // read/write error flags
    // UART_Print_Hex(mem_to_bus((uint32_t)&ring_buffer[0][0]));


    // for (int k = 0; k < BUFFER_SIZE; k++) {      
    //     UART_Send_Int(k);
    //     UART_Print_Hex(ring_buffer[0][k]);
    // }

    // uint8_t data[SAMPLES_PER_PACKET];
    // while (1) {
    //     // uint8_t buf[60];
    //     uint8_t buf[1];
    //     // for(int i = 0; i < 60; i++) buf[i] = i + 1;
    //     // CC1101_Send_Packet(&radio, buf, sizeof(buf));
        
    //     GPIO_Toggle(&josh);
    //     CC1101_Send_Packet(&radio, buf, sizeof(buf));
    // }

    return 0;
}
