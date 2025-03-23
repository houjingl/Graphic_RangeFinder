#include "sccb.h"

volatile int* sccb_JP2_port_addr = (int*) JP2_BASE;

void SDA_set_high()
{
    *sccb_JP2_port_addr |= (1 << SDA);
    usleep(50); // 50 us delay
}

void SDA_set_low()
{
    *sccb_JP2_port_addr &= ~(1 << SDA);
    usleep(50);
}

void SCL_set_high()
{
    *sccb_JP2_port_addr |= (1 << SCL);
    usleep(50);
}

void SCL_set_low()
{
    *sccb_JP2_port_addr &= ~(1 << SCL);
    usleep(50);
}

//Setting SCCB comm logic functions

void SCCB_start()
{
    //when SCL is high, drive SDA from High to low
    SDA_set_high();
    SCL_set_high();
    SDA_set_low();
    SCL_set_low();
}

void SCCB_stop()
{
    //when SCL is high, drive SDA from low to high
    SDA_set_low();
    SCL_set_high();
    SDA_set_high();
}

void SCCB_sendNoAck()
{
    SDA_set_high();
    SCL_set_high();
    SCL_set_low();
    SDA_set_low();
}