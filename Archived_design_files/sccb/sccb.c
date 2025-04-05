#include "sccb.h"

volatile int* sccb_JP2_port_addr = (int*) JP2_BASE;

void delay_us(unsigned int microseconds) {
    volatile unsigned int i;
    while (microseconds--) {
        // For a 100 MHz clock, ideally, you need ~100 cycles for 1µs delay.
        // The following loop (with the NOP) is an approximation.
        for (i = 0; i < 100; i++) {
            __asm__("nop");
        }
    }
}

//SCCB SCL SDA basic setup
void SCCB_port_init()
{
    *(sccb_JP2_port_addr + 1) |= (1 << SCL) | (1 << SDA); //set SDA and SCL to output
}

void SDA_set_high()
{
    *sccb_JP2_port_addr |= (1 << SDA);
    delay_us(50); // 50 us delay
}

void SDA_set_low()
{
    *sccb_JP2_port_addr &= ~(1 << SDA);
    delay_us(50);
}

void SCL_set_high()
{
    *sccb_JP2_port_addr |= (1 << SCL);
    delay_us(50);
}

void SCL_set_low()
{
    *sccb_JP2_port_addr &= ~(1 << SCL);
    delay_us(50);
}

void SDA_set(uint8_t vlevel)
{
    if(vlevel){
        SDA_set_high();
    } else {
        SDA_set_low();
    }
}

void SCL_set(uint8_t vlevel)
{
    if (vlevel){
        SCL_set_high();
    } else {
        SCL_set_low();
    }
}

//Setting SCCB comm functions
void SCCB_start()
{
    //when SCL is high, drive SDA from High to low
    //Drive SCL to low at the end of call
    SDA_set_high();
    SCL_set_high();
    SDA_set_low();
    SCL_set_low();
}

void SCCB_stop()
{
    //when SCL is high, drive SDA from low to high
    //Drive SCL to high at the end of call
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

uint8_t SCCB_Read_Ack()
{
    *(sccb_JP2_port_addr + 1) &= ~(1 << SDA); //set SDA to input, release SDA line
    SCL_set_high();
    uint8_t ack = (*sccb_JP2_port_addr & (1 << SDA)) >> SDA;
    SCL_set_low();
    *(sccb_JP2_port_addr + 1) |= (1 << SDA); //reset SDA to output/////////////////////////////////////might have issue
    return ack;
}

uint8_t SCCB_SendByte(uint8_t data)
{
    //SDA stable at SCL high, change at SCL low
    //SCL is set to low at the end of each function, no need to config SCL level at the start
    for (int i = 0; i < 8; i ++){
        //set SDA
        SDA_set((data >> i) & 0x1); //or data & (1 << i)
        SCL_set_high();
        SCL_set_low();
    }
    
    return SCCB_Read_Ack();
}

uint8_t SCCB_ReadByte()
{
    *(sccb_JP2_port_addr + 1) &= ~(1 << SDA); //set SDA to input, release SDA line
    uint8_t retByte = 0;
    for (int i = 0; i < 8; i ++){
        SCL_set_high();
        if (*sccb_JP2_port_addr & (1 << SDA)) retByte |= (1 << i);
        SCL_set_low();
    }
    *(sccb_JP2_port_addr + 1) |= (1 << SDA); 

    return retByte;
}

uint8_t SCCB_Reg_Write(uint8_t data, uint8_t reg_addr)
{
    SCCB_start();
    if(SCCB_SendByte(OV7670_write_addr)){
        SCCB_stop();
        return 0xFF; //0xFF indicates no ack from slave, comm fails
    }
    if(SCCB_SendByte(reg_addr)){
        SCCB_stop();
        return 0xFF; //0xFF indicates no ack from slave, comm fails
    }
    if(SCCB_SendByte(data)){
        SCCB_stop();
        return 0xFF; //0xFF indicates no ack from slave, comm fails
    }
    SCCB_stop();
    return 0;
}

uint8_t SCCB_Reg_Read(uint8_t reg_addr)
{   
    uint8_t regContent = 0;
    //SCCB read: two stages
    //1 stage: write reg addr; 
    SCCB_start();
    if(SCCB_SendByte(OV7670_write_addr)){
        SCCB_stop();
        return 0xFF;
    }
    if(SCCB_SendByte(reg_addr)){
        SCCB_stop();
        return 0xFF;
    }
    SCCB_stop();

    //2 stage: read reg content
    SCCB_start();
    if(SCCB_SendByte(OV7670_read_addr)){
        SCCB_stop();
        return 0xFF;
    }
    regContent = SCCB_ReadByte();
    SCCB_sendNoAck();
    SCCB_stop();
    return regContent;
}


