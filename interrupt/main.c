#define TIMER2_BASE             0xFF202020
#define LED_base                0xFF200000
#define TIMER2_IRQ              17
#define TS                      2000

int count = 0;
double pwm_duty_ratio = 0.5; //default 50% duty ratio
int pulse_width = 0;

void timer2_start()
{
    int* timer2_base = (int*) TIMER2_BASE;
    *timer2_base = 0x0;
    *(timer2_base + 1) = 0b0111;
}

void timer2_stop()
{
    int* timer2_base = (int*) TIMER2_BASE;
    *(timer2_base + 1) = 0b1011;
}

void timer2_ISR()
{
    timer2_stop();
    volatile int* led = (int*) LED_base;
    //PWM generator
    count ++;
    count %= TS;
    if (count < pulse_width){
        //set corresponding pin to 1
        *led = -1;
    } else {
        //set corresponding pin to 0
        *led = 0;
    }
    timer2_start();
    
}

void ISR_HANDLER()
{
    int mcause_value = 0x0;
    __asm__ volatile ("csrr %0, mcause" : "=r"(mcause_value));
    mcause_value &= ~(1 << 31);
    if (mcause_value == TIMER2_IRQ){
        timer2_ISR();
        timer2_start();
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

    mtvec_value = (int) &ISR_HANDLER; //load mtvec
    __asm__ volatile ("csrw mtvec, %0" :: "r"(mtvec_value));

    //enable global interrupt
    __asm__ volatile ("csrs mstatus, %0" :: "r"(mstatus_value));
}  

void timer2_init()
{
    int* timer2_base = (int*) TIMER2_BASE;
    int timer2_content = 1000;
    *(timer2_base) = 0x0;
    *(timer2_base + 1) = 0b1011;
    *(timer2_base + 2) = timer2_content;
    *(timer2_base + 3) = 0x0;
    timer2_interrupt_init();
}

int main(){
    timer2_init();
    timer2_start();

    while(1){
        for (int i = 0; i < TS; i ++){
            pulse_width = i;
            for (int j = 0; j < 30000; j ++){}
        }
    }
}

