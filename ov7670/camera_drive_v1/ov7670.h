#ifndef __OV7670D__
#define __OV7670D__

#include "sccb.h"
#ifndef JP2_BASE
#define JP2_BASE                0xFF200070
#endif

#define XCLK_TIMER_BASE         0xFF202000

#ifndef SDA
#define SDA                     0
#define SCL                     1
#endif

#define VSYNC                   2 //Vertical Sync: set to input
#define HSYNC                   3 //Horizontal Sync: set to input
#define PCLK                    4 //Pixel Clock: Set to input
#define XCLK                    5 //Camera System Clock: Set to output, outputing a stable 16Mhz square wave
#define RSTN                    6 //Set to output 0 -> reset all reg to default 1 -> Normal mode
#define PWDN                    7 //Set to output 0 -> working 1 -> idle

#define GETD7_D0                0xFF00  //Set to input
                                        //connect D0 to Pin#D8 on the DE1_Soc, connect D7 to Pin#D15 on the DE1_Soc, connect the rest in sequence!!

#endif