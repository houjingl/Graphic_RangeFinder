#ifndef __ULTRASONIC__
#define __ULTRASONIC__
//DE1 Soc equips with two extension ports - JP1 and JP2
//Each corresponds with a 32-bit Register
//JP1 is 0xFF200060, and for JP2 is 0xFF200070
/*
Base        Data bits
+4          Direction Bits
+8          Interrupt Mask
+C          Edge Capture bits
*/
//Each bit in the Data register can be configured as an input by setting the corresponding bit in the Direction register to 0 
//or as an output by setting this bit position to 1
#define JP1_BASE                0xFF200060
#define JP2_BASE                0xFF200070
#define TIMER1_BASE             0xFF202000
#define TIMER2_BASE             0xFF202020
#define HEX3_HEX0_BASE          0xFF200020
#define Trigger                 0
#define Echo                    1

#include <math.h>
#include <stdlib.h>

void ultrasonic_init();
void ultrasonic_send_wave();
double ultrasonic_compute_distance_cm();

//Should not call these functions outside of ultrasonic.c. still included in the .h file
void ultrasonic_CountDown_start();
void ultrasonic_count_10us();
void ultrasonic_timer_stop();

#endif