#define TIMER2_BASE             0xFF202020
#define LED_base                0xFF200000
#define TIMER2_IRQ              17
#define TS                      2000
#define Servo                   2            //using DP2

#ifndef JP1_BASE
#define JP1_BASE  0xFF200060
#endif

#ifndef KEY_BASE
#define KEY_BASE  0xFF200050
#define KEY_IRQ   18
#endif

int count = 0;
//int count2 = 0;
//double pwm_duty_ratio = 0.5; //default 50% duty ratio
volatile int* key_base = (int*) KEY_BASE;
volatile int pulse_width = 100;

void timer2_start();
void timer2_stop();
void timer2_ISR();
void __attribute__((interrupt("machine"))) ISR_HANDLER(void);
void timer2_interrupt_init();
void timer2_init();
void key_interrupt_init();
void key_init();
void key_ISR();


void timer2_start()
{
    int* timer2_base = (int*) TIMER2_BASE;
    *timer2_base = 0x0;
    *(timer2_base + 1) = 0b0111;
}

void timer2_stop()
{
    int* timer2_base = (int*) TIMER2_BASE;
    *timer2_base = 0x0;
    *(timer2_base + 1) = 0b1010;
}

void timer2_ISR()
{
    timer2_stop();
    volatile int* jp1 = (int*) JP1_BASE;
    //PWM generator
    count ++;
    count %= TS;
    if (count < pulse_width){
        //set corresponding pin to 1
        *jp1 |= (1<<Servo);
    } else {
        //set corresponding pin to 0
        *jp1 &= ~(1<<Servo);
    }
    timer2_start();
    // count2 ++;
    // count2 %= (TS*100);
    // if (count2 <(TS*50)){
    // 	pulse_width = 100;
    // }else {
    // 	pulse_width = 200;
    // }
}

void __attribute__((interrupt("machine"))) ISR_HANDLER(void) //declare this subroutine as an ISR
{
    int mcause_value = 0x0;
    __asm__ volatile ("csrr %0, mcause" : "=r"(mcause_value));
    if (mcause_value == (TIMER2_IRQ + 0x80000000)){
        timer2_ISR();
    }

}

void timer2_interrupt_init()
{   
    //THIS FUNCTION ONLY OPERATES THE GENERAL INTERRUPT CONTROL REGs
    volatile int* timer2_base = (int*) TIMER2_BASE;
    volatile int timer2_IRQ = TIMER2_IRQ;
    int mstatus_value, IRQ_value, mtvec_value;
    mstatus_value = 0b1000; //disable global interrupt
    __asm__ volatile ("csrc mstatus, %0" :: "r"(mstatus_value));

    IRQ_value |= (1 << timer2_IRQ); //set timer2 IRQ to mie
    __asm__ volatile ("csrs mie, %0" :: "r"(IRQ_value));

    key_interrupt_init(); // set key IRQ to mie

    mtvec_value = (int) &ISR_HANDLER; //load mtvec
    __asm__ volatile ("csrw mtvec, %0" :: "r"(mtvec_value));

    //enable global interrupt
    __asm__ volatile ("csrs mstatus, %0" :: "r"(mstatus_value));
}  

void timer2_init()
{
    int* timer2_base = (int*) TIMER2_BASE;
    int timer2_content = 1000;//10 us
    *(timer2_base) = 0x0;
    *(timer2_base + 1) = 0b1011;
    *(timer2_base + 2) = timer2_content;
    *(timer2_base + 3) = timer2_content >> 16;
    timer2_interrupt_init();
}

void key_interrupt_init(){
    volatile int key_IRQ = KEY_IRQ;
//mtvec is already set
    int IRQ_value;
    IRQ_value |= (1<<KEY_IRQ);
    __asm__ volatile ("csrs mie, %0" ::"r"(IRQ_value));
}

void key_init(){
    // key_interrupt_init is called in timer2_initial
    *(key_base+2) = 0b1111;
    *(key_base+3) = 0b1111;
}

void key_ISR(){
    int key;
    key = *(key_base+3);
    *(key_base+3) = *(key_base+3);//clear interrupt
    if (key == 0b0001){
        //R
        if (pulse_width<200){
            pulse_width += 5;
        }
    }else if (key == 0b0010){
        //L
        if (pulse_width>100){
            pulse_width -= 5;
        }
    }
}

int main(){
    timer2_init();
    timer2_start();
    key_init();
    volatile int* jp1Base = (int*) JP1_BASE;
    *(jp1Base+1) |= (1<<Servo);    
    while(1){

    }
}

