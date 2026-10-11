#pragma once

#include <stdint.h>
#include <stdbool.h>

#define MAX_PIN 53

#define BASE_ADDRESS 0x20200000

#define REG_SIZE_BITS 32

#define REG_SIZE_BYTES 4

#define PIN_MODE_BITS 3

#define GPIO_IRQ_BASE 49 - 32

enum {
    FSEL_BASE = BASE_ADDRESS,
    SET_BASE = BASE_ADDRESS + 0x1C,
    CLR_BASE = BASE_ADDRESS + 0x28,
    LEV_BASE = BASE_ADDRESS + 0x34,
    EDS_BASE = BASE_ADDRESS + 0x40,
    REN_BASE = BASE_ADDRESS + 0x4C,
    FEN_BASE = BASE_ADDRESS + 0x58,
    HEN_BASE = BASE_ADDRESS + 0x64,
    LEN_BASE = BASE_ADDRESS + 0x70,
    AREN_BASE = BASE_ADDRESS + 0x7C,
    AFEN_BASE = BASE_ADDRESS + 0x88,
    PUD_BASE = BASE_ADDRESS + 0x94,
    PUPDCLK_BASE = BASE_ADDRESS + 0x98,
};

typedef enum Pin_Output_t {
    LOW = 0,
    HIGH
} Pin_Output;

typedef enum Pin_Mode_t {
    INPUT = 0,
    OUTPUT = 1,
    AF0 = 4,
    AF1 = 5,
    AF2 = 6,
    AF3 = 7,
    AF4 = 3,
    AF5 = 2,
} Pin_Mode;

typedef enum Pin_PUPD_t {
    OFF = 0,
    PULL_DOWN,
    PULL_UP
} Pin_PUPD;

typedef enum Pin_Edge_t {
    RISING = 1,
    FALLING,
    ALL
} Pin_Edge;

typedef struct Pin_t {
    uint32_t pin_num;
    Pin_Output state;
} Pin;


void GPIO_Set(Pin *pin, Pin_Output value);

void GPIO_Set_High(Pin *pin);

void GPIO_Set_Low(Pin *pin);

void GPIO_Toggle(Pin *pin);

Pin_Output GPIO_Read(Pin *pin);

void GPIO_Config(Pin *pin, Pin_Mode mode);

void GPIO_Enable_IRQ(Pin *pin, Pin_Edge edge);

void GPIO_Enable_General_IRQ(Pin *pin);

bool GPIO_Check_Pin_IRQ(Pin *pin);

bool GPIO_Check_Any_Pin_IRQ();

void GPIO_Clear_IRQ(Pin *pin);
