#include "ultrasonic.h"

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

int main(void){
    ultrasonic_init();
    volatile int* seg_base = (int*) HEX3_HEX0_BASE;

    while (1){
        ultrasonic_send_wave();
        int distance = ultrasonic_compute_distance_cm();
        unsigned int segDisplay = 0x0;
        int i = 0;
        while (i != 32){
            unsigned char lsd = distance % 10;
            segDisplay += (signArray[lsd] << i);
            i += 8;
            distance /= 10;
        }
        *seg_base = segDisplay;

        for (int j = 0; j < 100000000; j ++){/* A very simple 1s delay*/}
    }
}