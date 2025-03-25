#include "pwm.h"

int main(){
    servo_pwm_init();
    servo_pwm_start();
    volatile int* jp1Base = (int*) JP1_BASE;
    *(jp1Base + 1) |= (1 << Servo);  
    int mtvec_value;
    mtvec_value = (int) &ISR_HANDLER; //load mtvec
    __asm__ volatile ("csrw mtvec, %0" :: "r"(mtvec_value));  
    while(1){

    }
}


void __attribute__((interrupt("machine"))) ISR_HANDLER(void) //declare this subroutine as an ISR
{
    int mcause_value = 0x0;
    __asm__ volatile ("csrr %0, mcause" : "=r"(mcause_value));
    if (mcause_value == (servo_pwm_IRQ + 0x80000000)){
        servo_pwm_ISR();
    }

}

