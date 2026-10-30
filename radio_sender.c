#include "miniuart.h"
#include <stdint.h>
#include "interrupts.h"
#include "hardware.h"
#include "utils.h"
#include "timer.h"

// #define MULTIPLICITY       20   // number of buffers in the ring
#define SAMPLES_PER_PACKET 60   // 60 x 1 byte (8-bit samples) = 60-byte packet
// #define DECIMATE           1    // average this many mic samples into one sent sample (1 = send every sample)

// // ring of buffers. the interrupt fills buffers[write_head], main sends buffers[read_head].
// // empty when read_head == write_head, full when the next write_head would equal read_head
// static volatile uint8_t buffers[MULTIPLICITY][SAMPLES_PER_PACKET];   // 8-bit signed samples
// static volatile uint8_t write_head = 1;   // only the interrupt changes this
// static volatile uint8_t read_head = 0;    // only main changes this
// static volatile uint32_t overruns = 0;    // buffers dropped because main fell behind

// static uint8_t sample_i = 0;              // position in buffers[write_head]
// static int32_t acc = 0;                   // running sum for DECIMATE
// static uint8_t acc_n = 0;

void __attribute__((interrupt("IRQ"))) interrupt_vector(void) {
    if (IRQ_Check_Interrupt(IRQ_PCM_INT)) {
        // GPIO_Toggle(&josh);
        uint32_t raw = I2S_Read_Value();
        I2S_Clear_IRQ();
    }

    //     acc += (int32_t)(raw << 8) >> 8;   // sign-extend the 24-bit sample before adding
    //     if (++acc_n < DECIMATE) {
    //         return;
    //     }
    //     // keep only the top 8 bits of the 24-bit sample (bits 23:16), as a signed byte
    //     buffers[write_head][sample_i] = (uint8_t)((acc / DECIMATE) >> 16);
    //     acc = 0;
    //     acc_n = 0;

    //     if (++sample_i == SAMPLES_PER_PACKET) {
    //         sample_i = 0;
    //         uint8_t next = (write_head + 1) % MULTIPLICITY;
    //         if (next != read_head) {
    //             write_head = next;   // hand the full buffer to main
    //         } else {
    //             overruns++;          // ring full: refill this buffer, dropping it
    //             UART_Send_Int(overruns);
    //         }
    //     }
    // }
}

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
    I2S_Enable_IRQ();

    SPI_Config(&spi);
    CC1101_Init(&radio, &CC1101_PRESET_250K);

    // interrupts_on();

    // uint8_t data[SAMPLES_PER_PACKET];
    while (1) {
        // if (read_head == write_head) {
        //     UART_Print_Hex(0xdead);
        //     continue;   // nothing full yet
        // }
        // GPIO_Set_High(&josh);
        // one byte per sample, so just copy. the interrupt won't touch this buffer until read_head moves
        // volatile uint8_t *b = buffers[read_head];
        // for (int k = 0; k < SAMPLES_PER_PACKET; k++) {
        //     data[k] = b[k];
        //     // data[k] = k + 1;
        // }
        // uint8_t buf[60];
        uint8_t buf[1];
        // for(int i = 0; i < 60; i++) buf[i] = i + 1;
        // CC1101_Send_Packet(&radio, buf, sizeof(buf));
        
        GPIO_Toggle(&josh);
        CC1101_Send_Packet(&radio, buf, sizeof(buf));

        // read_head = (read_head + 1) % MULTIPLICITY;
        // GPIO_Set_Low(&josh);
        // GPIO_Toggle(&josh);
    }

    return 0;
}
