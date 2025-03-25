#include "ov7670.h"

volatile int* ov7670_JP2_addr = (int*) JP2_BASE;
//Camera interrupt init process
/*
config XCLK -> using timer 1, set a 16 mhz clock, enable timer1 interrupt
turn on PCLK, VS, HS interrupt mask
l
*/
void ov7670_interrupt_init()
{
    
}

//initialization process
/*
init all pins
reset camera
set camera XCLK timer output
turn on PCLK, VS, HS interrupt mask, and load 
*/


void ov7670_init()
{
    //pins config
    SCCB_port_init(); //init SCL and SDA to output pins
    uint32_t direction_reg_content = 0x0;
    //Output is 1, input is 0
    //configure output pins only
    direction_reg_content |= (1 << XCLK) | (1 << RSTN) | (1 << PWDN);
    *(ov7670_JP2_addr + 1) |= direction_reg_content;

    //Camera status reset
    *(ov7670_JP2_addr) &= ~(1 << RSTN);
    delay_us(50);
    *(ov7670_JP2_addr) |= (1 << RSTN);

    //Camera Start
    *(ov7670_JP2_addr) &= ~(1 << PWDN);

    //reg config
    SCCB_Reg_Write();
}