#include "pwm.h"

int servo_pwm_count = 0;
volatile int servo_pwm_pulse_width = 100; //must declare this variable as volatile to avoid compiler optimization

void servo_pwm_start()
{
    int* servo_pwm_base = (int*) servo_pwm_BASE;
    *servo_pwm_base = 0x0;
    *(servo_pwm_base + 1) = 0b0111;
}

void servo_pwm_stop()
{
    int* servo_pwm_base = (int*) servo_pwm_BASE;
    *servo_pwm_base = 0x0;
    *(servo_pwm_base + 1) = 0b1010;
}

void servo_pwm_ISR()
{
    servo_pwm_stop();
    volatile int* jp1 = (int*) JP1_BASE;
    //PWM generator
    servo_pwm_count ++;
    servo_pwm_count %= TS;
    if (servo_pwm_count < servo_pwm_pulse_width){
        //set corresponding pin to 1
        *jp1 |= (1 << Servo);
    } else {
        //set corresponding pin to 0
        *jp1 &= ~(1 << Servo);
    }
    servo_pwm_start();
}



void servo_pwm_interrupt_init()
{   
    //THIS FUNCTION ONLY OPERATES THE GENERAL INTERRUPT CONTROL REGs
    volatile int* servo_pwm_base = (int*) servo_pwm_BASE;
    volatile int servo_pwm_irq = servo_pwm_IRQ;
    int mstatus_value, IRQ_value, mtvec_value;
    mstatus_value = 0b1000; //disable global interrupt
    __asm__ volatile ("csrc mstatus, %0" :: "r"(mstatus_value));

    IRQ_value |= (1 << servo_pwm_IRQ); //set servo_pwm IRQ to mie
    __asm__ volatile ("csrs mie, %0" :: "r"(IRQ_value));
    //Loading mtvec in main
    //mtvec_value = (int) &ISR_HANDLER; //load mtvec
    //__asm__ volatile ("csrw mtvec, %0" :: "r"(mtvec_value));

    //enable global interrupt
    __asm__ volatile ("csrs mstatus, %0" :: "r"(mstatus_value));
}  

void servo_pwm_init()
{
    int* servo_pwm_base = (int*) servo_pwm_BASE;
    int servo_pwm_content = 1000;//10 us
    *(servo_pwm_base) = 0x0;
    *(servo_pwm_base + 1) = 0b1011;
    *(servo_pwm_base + 2) = servo_pwm_content;
    *(servo_pwm_base + 3) = servo_pwm_content >> 16;
    servo_pwm_interrupt_init();
}

/*
void __attribute__((interrupt("machine"))) ISR_HANDLER(void) //declare this subroutine as an ISR
{
    int mcause_value = 0x0;
    __asm__ volatile ("csrr %0, mcause" : "=r"(mcause_value));
    if (mcause_value == (servo_pwm_IRQ + 0x80000000)){
        servo_pwm_ISR();
    }

}
*/
int main(){
    servo_pwm_init();
    servo_pwm_start();
    volatile int* jp1Base = (int*) JP1_BASE;
    *(jp1Base+1) |= (1<<Servo);    
    while(1){

    }
}

