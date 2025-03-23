#ifndef __SCCB__
#define __SCCB__

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

//SCCB is the communication protocal used by OV7670. Very similiar to IIC.

#define JP2_BASE                0xFF200070
#define SDA                     0
#define SCL                     1
#define OV7670_write_addr       0x42
#define OV7670_read_addr        0x43

uint8_t SCCB_Reg_Write(uint8_t data, uint8_t reg_addr);
uint8_t SCCB_Reg_Read(uint8_t reg_addr);

#endif