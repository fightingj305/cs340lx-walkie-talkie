#include "miniuart.h"
#include <stdint.h>
#include "interrupts.h"
#include "hardware.h"
#include "utils.h"
#include "dma.h"
#include "rng.h"

DMA_SECTION DMA_CB dma_buf[16];
DMA_SECTION DMA_CB dma_buf2[16];
DMA_Write recv_writes[1024];
DMA_Write send_writes[1024];

#define BYTES_PER_SAMPLE 2
#define SAMPLES_PER_PACKET 30

#define BUFFER_SIZE 256
#define BUFFER_COUNT 16

volatile uint32_t recv_buffer[BUFFER_COUNT][BUFFER_SIZE];
uint32_t recv_head = 0;
uint32_t current_recv_buffer = BUFFER_COUNT - 1;

volatile uint32_t send_buffer[BUFFER_COUNT][BUFFER_SIZE];
uint32_t send_head = 0;
uint32_t current_send_buffer = BUFFER_COUNT - 1;

volatile uint32_t last_packet;
volatile bool started = false;
volatile bool received = false;

void __attribute__((interrupt("IRQ"))) interrupt_vector(void) { 
    if (GPIO_Check_Any_Pin_IRQ() && GPIO_Check_Pin_IRQ(&radio_b.gdo2)) {
        CC1101_RX_Status status;
        uint8_t mic_val[60];
        int retval = CC1101_Read_Packet(&radio_b, mic_val, 60, &status);
        if (status.crc_ok) {
            UART_Send_String("Received\n");
            received = true;
            if (!started) {
                DMA_Start(DMA_Channel_4);
                started = true;
            }
            for (uint32_t i = 0; i < 60; i+=2) {
                uint32_t value = mic_val[i] << 16 | mic_val[i+1] << 8;
                recv_buffer[current_recv_buffer][recv_head] = value;
                recv_head++;
                if (recv_head >= BUFFER_SIZE) {
                    recv_head = 0;
                    uint32_t next_buffer = (current_recv_buffer + 1) % BUFFER_COUNT;
                    while (DMA_Get_Current_Source(DMA_Channel_4) >= ((uint32_t)&recv_buffer[next_buffer][0]) &&
                        DMA_Get_Current_Source(DMA_Channel_4) <((uint32_t)&recv_buffer[next_buffer][BUFFER_SIZE])) {
                    }
                    current_recv_buffer = next_buffer;
                }
            }
            last_packet = TIM_SYS_Get_Ticks();
        }
        GPIO_Clear_IRQ(&radio_b.gdo2);
        // CC1101_Start_RX(&radio_b);
    }
    else {
        PANIC("Unexpected interrupt!");
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

    GPIO_Config(&josh, OUTPUT);

    interrupts_on();

    for (uint32_t i = 0; i < BUFFER_COUNT; i++) {
        recv_writes[i] = (DMA_Write) {
                .src = (uint32_t)&recv_buffer[i][0],
                .dst = PCM_FIFO_A,
                .size = 4 * BUFFER_SIZE,
                .periph = DMA_DREQ_PCM_TX,
                .periph_read = false,
                .inc_src = true,
                .inc_dst = false
        };
    }

    for (uint32_t i = 0; i < BUFFER_COUNT; i++) {
        send_writes[i] = (DMA_Write) {
            .src = PCM_FIFO_A,
            .dst = (uint32_t)&send_buffer[i][0],
            .size = 4 * BUFFER_SIZE,
            .periph = DMA_DREQ_PCM_RX,
            .periph_read = true,
            .inc_src = false,
            .inc_dst = true
        };
    }


    DMA_Transmit(DMA_Channel_4, recv_writes, dma_buf, BUFFER_COUNT);
    DMA_Start(DMA_Channel_4);

    DMA_Transmit(DMA_Channel_5, send_writes, dma_buf2, BUFFER_COUNT);
    DMA_Start(DMA_Channel_5);
    started = true;

    while (1) {       
        if (started && TIM_SYS_Get_Ticks() > last_packet + 20000) {
            DMA_Pause(DMA_Channel_4);
            started = false;
        }

        uint8_t packet[SAMPLES_PER_PACKET * BYTES_PER_SAMPLE];

        static uint8_t i = 0;

        for(uint32_t packet_i = 0; packet_i < SAMPLES_PER_PACKET * BYTES_PER_SAMPLE; packet_i+= BYTES_PER_SAMPLE) {
            for(uint32_t sample_word_i = 0; sample_word_i < BYTES_PER_SAMPLE; sample_word_i++) {
                if(sample_word_i == 0) {
                    uint32_t word = send_buffer[current_send_buffer][send_head];
                    packet[packet_i] = (word >> 16) & 0xFF;
                    packet[packet_i + 1] = (word >> 8) & 0xFF;
                }

                send_head++;

                if (send_head >= BUFFER_SIZE) {
                    send_head = 0;
                    uint32_t next_buffer = (current_send_buffer + 1) % BUFFER_COUNT;
                    while (DMA_Get_Current_Destination(DMA_Channel_5) >= ((uint32_t) &send_buffer[next_buffer][0]) &&
                        DMA_Get_Current_Destination(DMA_Channel_5) < ((uint32_t) &send_buffer[next_buffer][BUFFER_SIZE]));
                    
                    current_send_buffer = next_buffer;
                }
            }
        }

        CC1101_Send_Packet(&radio_b, packet, SAMPLES_PER_PACKET * BYTES_PER_SAMPLE);
        UART_Send_String("Toggling main loop\n");
        received = false;
        uint32_t ticks = TIM_SYS_Get_Ticks();
        CC1101_Start_RX(&radio_b);
        uint32_t sleep_amt = RNG_Randint(10000, 25000);
        while (!received && TIM_SYS_Get_Ticks() - ticks < sleep_amt);
        i++;
    }
    return 0;
}