#include "gpio.h"
#include "gpclk.h"

#define PWM_BASE 0x2020C000

typedef enum {
    PWM_Channel1,
    PWM_Channel2
} PWM_Channel;

enum {
    PWM_CTL = PWM_BASE + 0x0,
    PWM_STA = PWM_BASE + 0x4,
    PWM_DMAC = PWM_BASE + 0x8,
    PWM_RNG1 = PWM_BASE + 0x10,
    PWM_DAT1 = PWM_BASE + 0x14,
    PWM_FIF1 = PWM_BASE + 0x18,
    PWM_RNG2 = PWM_BASE + 0x20,
    PWM_DAT2 = PWM_BASE + 0x24,
};

enum {
    PWM_CTL_PWEN1 = 1 << 0,
    PWM_CTL_PWEN2 = 1 << 8
};


void PWM_Config(PWM_Channel channel, GPCLK *clk, Pin *pin);
void PWM_Set_Duty(PWM_Channel channel, uint32_t duty_percent);