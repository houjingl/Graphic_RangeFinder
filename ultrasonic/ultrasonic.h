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

#define JP1_BASE                0xFF200060
#define JP2_BASE                0xFF200070
#define TIMER1_BASE             0xFF202000
#define TIMER2_BASE             0xFF202020



#endif