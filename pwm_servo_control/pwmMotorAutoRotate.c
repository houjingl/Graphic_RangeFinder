#define JP1_BASE                0xFF200060
#define JP2_BASE                0xFF200070
#define TIMER1_BASE             0xFF202000
#define TIMER2_BASE             0xFF202020
#define HEX3_HEX0_BASE          0xFF200020
#define Trigger                 0
#define Echo                    1

#define SEG_0 0x3F  // 0b00111111: segments 0..5 on, 6 off
#define SEG_1 0x06  // 0b00000110: segments 1,2 on
#define SEG_2 0x5B  // 0b01011011
#define SEG_3 0x4F  // 0b01001111
#define SEG_4 0x66  // 0b01100110
#define SEG_5 0x6D  // 0b01101101
#define SEG_6 0x7D  // 0b01111101
#define SEG_7 0x07  // 0b00000111
#define SEG_8 0x7F  // 0b01111111
#define SEG_9 0x6F  // 0b01101111

volatile unsigned int distance;
unsigned char signArray[] = {SEG_0, SEG_1, SEG_2, SEG_3, SEG_4, SEG_5, SEG_6, SEG_7, SEG_8, SEG_9};

void ultrasonic_init()
{
    //Set up JP1 Trig bit as output, Echo bit as input
    volatile int* parallel_port1_base = (int*) JP1_BASE;
    *(parallel_port1_base + 1) = 0x1;
}

void ultrasonic_CountDown_start()
{
    volatile int* timer_base = (int*) TIMER1_BASE;
    //Fill up timer content
    *(timer_base + 2) = 0xffff;
    *(timer_base + 3) = 0xffff;
    *(timer_base) = 0x0; //clear TO
    *(timer_base + 1) = 0b0110; //set to continuous count mode. Disable interrupt
}

void ultrasonic_count_10us()
{
    volatile int* timer_base = (int*) TIMER1_BASE;
    int timer_content = 1000;
    *(timer_base + 2) = timer_content;
    *(timer_base + 3) = 0x0;
    *(timer_base) = 0x0; //clear TO
    *(timer_base + 1) = 0b0110; //set to continuous count mode. Disable interrupt
}

void ultrasonic_timer_stop()
{
    volatile int* timer_base = (int*) TIMER1_BASE;
    *(timer_base + 1) = 0b1010;
}

void ultrasonic_send_wave()
{
    volatile int* parallel_port1_base = (int*) JP1_BASE;
    volatile int* timer_base = (int*) TIMER1_BASE;
    *(parallel_port1_base) |= (1 << Trigger);
    ultrasonic_count_10us();
    while((*timer_base & 0x1) == 0);
    ultrasonic_timer_stop();
    *(parallel_port1_base) &= ~(1 << Trigger);
}

int ultrasonic_compute_distance_cm()
{
    volatile int* parallel_port1_base = (int*) JP1_BASE;
    volatile int* timer_base = (int*) TIMER1_BASE;
    volatile int* store = 0x20000;
    double distance = 0;
    while(((*parallel_port1_base & (1 << Echo)) >> Echo) == 0); //wait when echo is not received;
    ultrasonic_CountDown_start();
    while((*parallel_port1_base & (1 << Echo)) >> Echo); //wait when echo is high
    *(timer_base + 4) = 0x1;
    unsigned int timer_current_low = *(timer_base + 4);
    unsigned int timer_current_high = *(timer_base + 5);
    ultrasonic_timer_stop();
    unsigned int timer_current = timer_current_low + (timer_current_high << 16);
    *(store + 2) = timer_current;
    *(store + 3) = *(timer_base + 2) + ((*(timer_base + 3) << 16));
    distance = ((0xFFFFFFFF - timer_current) / 100000000.0) * (170) * 100.0; 
    //Distance in cm
    return (int) distance; 
    //round to 1 decimal places
}

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

volatile int count = 0;
volatile int count_rotate = 0;
volatile int fbi = 1;
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
    count_rotate ++;
    count_rotate %= TS * 2;
    if(!count_rotate){
        ultrasonic_send_wave();
        
        volatile int* led_base = 0xFF200000;
        *led_base ^= 0x1;
    	if(pulse_width <= 50) {
    	 	fbi = 1;
    	} else if (pulse_width >= 240){
    		fbi = -1;
    	}
    	pulse_width += fbi;
    }
    if (count < pulse_width){
        //set corresponding pin to 1
        *jp1 |= (1<<Servo);
    } else {
        //set corresponding pin to 0
        *jp1 &= ~(1<<Servo);
    }
    timer2_start();
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


int main(){
    timer2_init();
    timer2_start();
    volatile int* seg_base = (int*) HEX3_HEX0_BASE;
    volatile int* jp1Base = (int*) JP1_BASE;
    *(jp1Base+1) |= (1<<Servo);    
    while(1){
        unsigned int segDisplay = 0x0;
        int i = 0;
        while (i != 32){
            unsigned char lsd = distance % 10;
            segDisplay += (signArray[lsd] << i);
            i += 8;
            distance /= 10;
        }
        *seg_base = segDisplay;

    }
}

