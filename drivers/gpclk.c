#include "gpclk.h"
#include "utils.h"

CM_Reg GPCLK_Get_Ctl(GPCLK *clk) {
    switch (clk->clk_periph)
    {
    case GPCLK0:
        return GPCLK_GP0CTL;
    case GPCLK1:
        return GPCLK_GP1CTL;
    case GPCLK2:
        return GPCLK_GP2CTL;
    case PCMCLK:
        return GPCLK_PCMCTL;
    case PWMCLK:
        return GPCLK_PWMCTL;
    default:
        return -1;
    }
}

CM_Reg GPCLK_Get_Div(GPCLK *clk) {
    switch (clk->clk_periph)
    {
    case GPCLK0:
        return GPCLK_GP0DIV;
    case GPCLK1:
        return GPCLK_GP1DIV;
    case GPCLK2:
        return GPCLK_GP2DIV;
    case PCMCLK:
        return GPCLK_PCMDIV;
    case PWMCLK:
        return GPCLK_PWMDIV;
    default:
        return -1;
    }
}

void GPCLK_Enable(GPCLK *clk) {
    CM_Reg clk_ctl = GPCLK_Get_Ctl(clk);
    uint32_t old_value = GET32(clk_ctl);
    PUT32(clk_ctl, GPCLK_PASSWD | old_value | GPCLK_CTL_ENAB);
    while (!(GET32(clk_ctl) & GPCLK_CTL_BUSY));
}

void GPCLK_Disable(GPCLK *clk) {
    CM_Reg clk_ctl = GPCLK_Get_Ctl(clk);
    uint32_t old_value = GET32(clk_ctl);
    PUT32(clk_ctl, GPCLK_PASSWD | (old_value & ~GPCLK_CTL_ENAB));
    while (GET32(clk_ctl) & GPCLK_CTL_BUSY);
}

void GPCLK_Init(GPCLK *clk) {
    CM_Reg clk_ctl = GPCLK_Get_Ctl(clk);
    CM_Reg clk_div = GPCLK_Get_Div(clk);
    
    GPCLK_Disable(clk);

    uint32_t source_speed;
    switch (clk->source)
    {
    case GPCLK_SRC_PLLD:
        source_speed = PLLD_Freq;
        break;
    case GPCLK_SRC_PLLC:
        source_speed = PLLC_Freq;
        break;
    case GPCLK_SRC_OSC:
        source_speed = OSC_Freq;
        break;
    default:
        source_speed = PLLA_Freq;
        break;
    }
    uint32_t divi = source_speed / clk->speed;
    uint32_t divf = (uint32_t)(((uint64_t)(source_speed % clk->speed) * 4096) / clk->speed);
    
    ASSERT(divi >= 2);

    PUT32(clk_div,
        GPCLK_PASSWD |
        (divi << GPCLK_DIV_DIVI_SHIFT) |
        (divf << GPCLK_DIV_DIVF_SHIFT));

    PUT32(clk_ctl, GPCLK_PASSWD | clk->source | GPCLK_CTL_MASH_1);
    GPCLK_Enable(clk);
}

void GPCLK_Pin_Init(GPCLK *clk) {
    ASSERT(clk->clk_periph != PCMCLK && clk->clk_periph != PWMCLK);
    switch (clk->clk_periph)
    {
    case GPCLK0:
        clk->output_pin.pin_num = 4;
        break;
    case GPCLK1:
        clk->output_pin.pin_num = 5;
        break;
    case GPCLK2:
        clk->output_pin.pin_num = 6;
        break;
    default:
        clk->output_pin.pin_num = -1;
        break;
    }

    GPIO_Config(&clk->output_pin, AF0);
}