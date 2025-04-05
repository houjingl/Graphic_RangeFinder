#include "ultrasonic.h"

void ultrasonic_init()
{
    //Set up JP1 Trig bit as output, Echo bit as input
    volatile int* parallel_port1_base = (int*) JP1_BASE;
    *(parallel_port1_base + 1) |= (1 << Trigger); //set 1
    *(parallel_port1_base + 1) &= ~(1 << Echo); //set 0
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
    double distance = 0;
    while(((*parallel_port1_base & (1 << Echo)) >> Echo) == 0); //wait when echo is not received;
    ultrasonic_CountDown_start();
    while(((*parallel_port1_base & (1 << Echo)) >> Echo)); //wait when echo is high
    *(timer_base + 4) = 0x1; //perform write operation to store in snopshot
    unsigned int timer_current_low = *(timer_base + 4);
    unsigned int timer_current_high = *(timer_base + 5);
    ultrasonic_timer_stop();
    unsigned int timer_current = timer_current_low + timer_current_high << 16;
    distance = ((0xFFFFFFFF - timer_current) / 100000000) * (170) * 100.0; //Distance in cm
    return (int) distance; 
    //round to 1 decimal places
}