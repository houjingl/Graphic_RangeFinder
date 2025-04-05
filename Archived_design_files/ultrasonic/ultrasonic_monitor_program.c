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

int main(void){
    ultrasonic_init();
    volatile int* seg_base = (int*) HEX3_HEX0_BASE;
    volatile int* led_base = 0xFF200000;
    volatile int* store = 0x20000;
    while (1){
        *led_base ^= 0x1;
        ultrasonic_send_wave();
        unsigned int distance = ultrasonic_compute_distance_cm();
        *(store + 1) = distance;
        unsigned int segDisplay = 0x0;
        int i = 0;
        while (i != 32){
            unsigned char lsd = distance % 10;
            segDisplay += (signArray[lsd] << i);
            i += 8;
            distance /= 10;
        }
        *seg_base = segDisplay;

        for (int j = 0; j < 1000; j ++){/* A very simple 1s delay*/}
    }
}