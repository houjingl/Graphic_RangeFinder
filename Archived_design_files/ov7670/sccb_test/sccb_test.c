#include "sccb.h"

int main(){
    uint16_t pid = 0;
    uint16_t mid = 0;

    SCCB_port_init();
    pid = (SCCB_Reg_Read(0x0A) << 8) | (SCCB_Reg_Read(0x0B));
    mid = (SCCB_Reg_Read(0x1C) << 8) | (SCCB_Reg_Read(0x1D));
    
    volatile int* temp_store = (int*) 0x20000;
    *(temp_store + 1) = pid;
    *(temp_store + 2) = mid;
    while (1){

    }
}