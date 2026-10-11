#include "pwm.h"
#include "utils.h"

void PWM_Config(PWM_Channel channel, GPCLK *clk, Pin *pin) {
    switch (channel) {
        case PWM_Channel1:
            if (pin->pin_num == 12) {
                GPIO_Config(pin, AF0);
                break;
            }
            else if (pin->pin_num == 18) {
                GPIO_Config(pin, AF5);
                break;
            }
            else if (pin->pin_num == 40) {
                GPIO_Config(pin, AF0);
                break;
            }
            else if (pin->pin_num == 52) {
                GPIO_Config(pin, AF1);
                break;
            }
        case PWM_Channel2:
            if (pin->pin_num == 13) {
                GPIO_Config(pin, AF0);
                break;
            }
            else if (pin->pin_num == 19) {
                GPIO_Config(pin, AF5);
                break;
            }
            else if (pin->pin_num == 41) {
                GPIO_Config(pin, AF0);
                break;
            }
            else if (pin->pin_num == 45) {
                GPIO_Config(pin, AF0);
                break;
            }
            else if (pin->pin_num == 53) {
                GPIO_Config(pin, AF1);
                break;
            }
        default:
            PANIC("Invalid PWM pin configuration\n");
            break;
    }
    RMW_OR(PWM_CTL, channel == PWM_Channel1 ? PWM_CTL_PWEN1 : PWM_CTL_PWEN2);
    GPCLK_Init(clk);
}

void PWM_Set_Duty(PWM_Channel channel, uint32_t duty_percent) {
    switch (channel) {
        case PWM_Channel1:
            PUT32(PWM_RNG1, 100);
            PUT32(PWM_DAT1, duty_percent);
            break;
        case PWM_Channel2:
            PUT32(PWM_RNG2, 100);
            PUT32(PWM_DAT2, duty_percent);
            break;
        default:
            PANIC("Invalid Channel");
    }
}