#include "gpio.h"
#include "utils.h"
#include "interrupts.h"

void GPIO_Set(Pin *pin, Pin_Output value) {
    value == HIGH ? GPIO_Set_High(pin) : GPIO_Set_Low(pin);
}

void GPIO_Set_High(Pin *pin) {
    ASSERT(pin->pin_num <= MAX_PIN);

    const uint32_t ADDR = SET_BASE + REG_SIZE_BYTES * (pin->pin_num / REG_SIZE_BITS);
    const uint32_t MASK = 1 << (pin->pin_num % REG_SIZE_BITS);
    PUT32(ADDR, MASK);

    pin->state = HIGH;
}

void GPIO_Set_Low(Pin *pin) {
    ASSERT(pin->pin_num <= MAX_PIN);

    const uint32_t ADDR = CLR_BASE + REG_SIZE_BYTES * (pin->pin_num / REG_SIZE_BITS);
    const uint32_t MASK = 1 << (pin->pin_num % REG_SIZE_BITS);
    PUT32(ADDR, MASK);

    pin->state = LOW;
}

void GPIO_Toggle(Pin *pin) {
    GPIO_Set(pin, pin->state == LOW ? HIGH : LOW);
}

Pin_Output GPIO_Read(Pin *pin) {
    ASSERT(pin->pin_num <= MAX_PIN);
    
    const uint32_t ADDR = LEV_BASE + REG_SIZE_BYTES * (pin->pin_num / REG_SIZE_BITS);
    const uint32_t MASK = 1 << (pin->pin_num % REG_SIZE_BITS);
    return  (Pin_Output) ((GET32(ADDR) & MASK) != 0);
}

void GPIO_Config(Pin *pin, Pin_Mode mode) {
    ASSERT(pin->pin_num <= MAX_PIN);

    const uint32_t MASK = (1 << PIN_MODE_BITS) - 1;
    const uint32_t PINS_PER_REG = REG_SIZE_BITS / PIN_MODE_BITS; // Should be ten
    const uint32_t AMOUNT_TO_SHIFT = (pin->pin_num % PINS_PER_REG) * PIN_MODE_BITS;
    const uint32_t FSEL_ADDR = FSEL_BASE + REG_SIZE_BYTES * (pin->pin_num / PINS_PER_REG);
    
    volatile uint32_t current_config = GET32(FSEL_ADDR);
    current_config &= ~(MASK << AMOUNT_TO_SHIFT);
    
    PUT32(FSEL_ADDR, current_config | mode << AMOUNT_TO_SHIFT);
    pin->state = LOW;
}

void GPIO_Enable_IRQ(Pin *pin, Pin_Edge edge) {
    if (edge & RISING) { 
        const uint32_t REN_ADDR = REN_BASE + REG_SIZE_BYTES * (pin->pin_num / REG_SIZE_BITS);
        const uint32_t MASK = 1 << (pin->pin_num % REG_SIZE_BITS);
        volatile uint32_t current_ren = GET32(REN_ADDR);
        PUT32(REN_ADDR, current_ren | MASK);
    }
    if (edge & FALLING) {
        const uint32_t FEN_ADDR = FEN_BASE + REG_SIZE_BYTES * (pin->pin_num / REG_SIZE_BITS);
        const uint32_t MASK = 1 << (pin->pin_num % REG_SIZE_BITS);
        volatile uint32_t current_ren = GET32(FEN_ADDR);
        PUT32(FEN_ADDR, current_ren | MASK);
    }
}

void GPIO_Enable_General_IRQ(Pin *pin) {
    const uint32_t IRQ_MASK = 1 << ((GPIO_IRQ_BASE + pin->pin_num / REG_SIZE_BITS) % REG_SIZE_BITS);
    volatile uint32_t current_state = GET32(IRQ_ENABLE_2);
    PUT32(IRQ_ENABLE_2, current_state | IRQ_MASK);
}

bool GPIO_Check_Any_Pin_IRQ() {
    const uint32_t PENDING_ADDR = IRQ_PENDING_2;
    const uint32_t MASK = 0b11 << (GPIO_IRQ_BASE);
    return (GET32(PENDING_ADDR) & MASK) != 0;
}

bool GPIO_Check_Pin_IRQ(Pin *pin) {
    const uint32_t EDS_ADDR = EDS_BASE + REG_SIZE_BYTES * (pin->pin_num / REG_SIZE_BITS);
    const uint32_t MASK = 1 << (pin->pin_num % REG_SIZE_BITS);
    return (GET32(EDS_ADDR) & MASK) != 0;
}

void GPIO_Clear_IRQ(Pin *pin) {
    const uint32_t EDS_ADDR = EDS_BASE + REG_SIZE_BYTES * (pin->pin_num / REG_SIZE_BITS);
    const uint32_t MASK = 1 << (pin->pin_num % REG_SIZE_BITS);
    PUT32(EDS_ADDR, MASK);
}