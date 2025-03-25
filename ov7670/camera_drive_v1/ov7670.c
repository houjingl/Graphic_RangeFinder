#include "ov7670.h"

volatile int* ov7670_JP2_addr = (int*) JP2_BASE;
//Camera interrupt init process
/*
config XCLK -> using timer 1, set a 16 mhz clock, enable timer1 interrupt
turn on PCLK, VS, HS interrupt mask
load JP2 interrput and timer 1 interrupt
*/
void ov7670_xclk_start()
{
    volatile int* xclk_base = (int*) XCLK_TIMER_BASE;
    *xclk_base = 0x0;
    *(xclk_base) = 0b0111;
}

void ov7670_xclk_stop()
{
    volatile int* xclk_base = (int*) XCLK_TIMER_BASE;
    *xclk_base = 0x0;
    *(xclk_base + 1) = 0b1010;
}

void ov7670_xclk_ISR()
{
    ov7670_xclk_stop();
    volatile int* xclk_base = (int*) XCLK_TIMER_BASE;
    //toggle XCLK
    if (*ov7670_JP2_addr & (1 << XCLK)){
        //if XCLK is 1
        *(ov7670_JP2_addr) &= ~(1 << XCLK);
    } else {
        //XCLK is 0
        *(ov7670_JP2_addr) |= (1 << XCLK);
    }
    ov7670_xclk_start();
}

void ov7670_interrupt_init()
{
    //interrupt device level set up
    //setting XCLK
    volatile int* xclk_base = (int*) XCLK_TIMER_BASE;
    volatile int xclk_irq = XCLK_IRQ;
    volatile int jp2_irq = JP2_IRQ;
    int xclk_timer1_content = 5; //every 5 clock cycles toggle
    *(xclk_base + 2) = xclk_timer1_content;
    *(xclk_base + 3) = 0x0;
    ov7670_xclk_start();

    //setting JP2 interrupt mask
    *(ov7670_JP2_addr + 2) |= (1 << PCLK) | (1 << HSYNC) | (1 << VSYNC);
    //clear edge capture, writing 1
    *(ov7670_JP2_addr + 3) |= 0xFFFFFFFF;
    
    int mstatus_value, IRQ_value, mtvec_value;
    mstatus_value = 0b1000; //disable global interrupt
    __asm__ volatile ("csrc mstatus, %0" :: "r"(mstatus_value));

    __asm__ volatile ("csrr %0, mie" : "=r"(IRQ_value));
    IRQ_value |= (1 << XCLK_IRQ) | (1 << JP2_IRQ);
    __asm__ volatile ("csrs mie, %0" :: "r"(IRQ_value));
    //Loading mtvec in main
    //mtvec_value = (int) &ISR_HANDLER; //load mtvec
    //__asm__ volatile ("csrw mtvec, %0" :: "r"(mtvec_value));

    //enable global interrupt
    __asm__ volatile ("csrs mstatus, %0" :: "r"(mstatus_value));
}

void ov7670_JP2_ISR()
{
    //Important ISR, Determine the pixel transmission
    /*
    General Steps:
    1. check which pin caused the interrupt
        VS -> Vsync is completed, ok to render on VGA display  
            -> write 1 into front buffer
            -> reset y pos and x pos to 0 0
        HS -> switch line y ++
           -> reset PCLK buffer index
        PCLk -> receive pixel info
             -> need a buffer of size >= 2
             -> PCLK buffer index ++
             if PCLK % 2 == 0
                -> pixel map to current x, y loc
                -> x ++
                -> PCLK %= 2
    2. reset curresponding edge trigger pin
    */
   
}

//initialization process
/*
init all pins
reset camera
set camera XCLK timer output
turn on PCLK, VS, HS interrupt mask, and load 
*/

void ov7670_init()
{
    //pins config
    SCCB_port_init(); //init SCL and SDA to output pins
    uint32_t direction_reg_content = 0x0;
    //Output is 1, input is 0
    //configure output pins only
    direction_reg_content |= (1 << XCLK) | (1 << RSTN) | (1 << PWDN);
    *(ov7670_JP2_addr + 1) |= direction_reg_content;

    //Camera status reset
    *(ov7670_JP2_addr) &= ~(1 << RSTN);
    delay_us(50);
    *(ov7670_JP2_addr) |= (1 << RSTN);

    //Camera Start
    *(ov7670_JP2_addr) &= ~(1 << PWDN);

    //reg config
    OV7670_Config_Reg();
}

void OV7670_Config_Reg(){
    /*0x03 0x11 0x12 0x17 0x18 0x19 0x1a 0x1e 0x32 0x40 0x6b 0x70 0x71 0x8c*/
    SCCB_Reg_Write(0x11, 0x40); // directly using external clk
	
    SCCB_Reg_Write(0x12, 0x14); // QVGA(320*240), RGB
    SCCB_Reg_Write(0x3e, 0x00); // COM14,dcw, pclk *default 0x00

    SCCB_Reg_Write(0x6b, 0x4A); // pll control

    SCCB_Reg_Write(0x8C, 0x00); // 失能RGB444
    SCCB_Reg_Write(0x3a, 0x00); // 负片失能; 使用通用UV输出;
    SCCB_Reg_Write(0x40, 0xD0); // 数据输出范围为00~FF; 使用RGB565
    SCCB_Reg_Write(0x0c, 0x0c); // COM3使能缩放

    SCCB_Reg_Write(0x4b, 0x01);	// UV平均 使能
    SCCB_Reg_Write(0x13, 0xff); // COM8 AGC,AWB,AEC使能控制
    SCCB_Reg_Write(0x1e, 0x37); // 水平镜像/竖直翻转使能 
    SCCB_Reg_Write(0x74, 0x19);	// 手动数字增益

    SCCB_Reg_Write(0x72, 0x11); // DCW 控制,默认值0x11
    SCCB_Reg_Write(0x73, 0x00); // DSP缩放时钟,默认值0x00
   
    OV7670_config_window(272,12,320,240);   // resolution 320*240
}

void OV7670_config_window(unsigned int startx,unsigned int starty,unsigned int width, unsigned int height){
	unsigned int endx;
	unsigned int endy;// "v*2"必须
	unsigned char temp_reg1, temp_reg2;
	unsigned char temp = 0;
	
	endx = (startx + width);
	endy = (starty + height + height);// "v*2"必须
    temp_reg1 = SCCB_Reg_Read(0x03);
	temp_reg1 &= 0xf0;
    temp_reg2 = SCCB_Reg_Read(0x32);
	temp_reg2 &= 0xc0;
	
	// Horizontal
	temp = temp_reg2 | ((endx & 0x7) << 3) | (startx & 0x7);
    SCCB_Reg_Write(0x32, temp);
	temp = (startx & 0x7F8) >> 3;
    SCCB_Reg_Write(0x17, temp);
	temp = (endx&0x7F8)>>3;
    SCCB_Reg_Write(0x18, temp );
	
	// Vertical
	temp = temp_reg1 | ((endy&0x3)<<2) | (starty&0x3);
    SCCB_Reg_Write(0x03, temp);
	temp = starty>>2;
    SCCB_Reg_Write(0x19, temp);
	temp = endy>>2;
    SCCB_Reg_Write(0x1A, temp);
}
