#define JP1_BASE 0xFF200060
#define JP2_BASE 0xFF200070
#define TIMER1_BASE 0xFF202000
#define TIMER2_BASE 0xFF202020
#define HEX3_HEX0_BASE 0xFF200020
#define Trigger 0
#define Echo 1

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

#ifndef VGA_controller
#define VGA_controller 0xff203020
#define PI 3.141592653589793
#endif

volatile int pixel_buffer_start;       // global variable
volatile short int Buffer1[240][512];  // 240 rows, 512 (320 + padding) columns
volatile short int Buffer2[240][512];

int calculate_x(int distance, int angle);
int calculate_y(int distance, int angle);
void plot_pixel(int x, int y, short int line_color);
void wait_for_vsync();
void clear_screen();
void swap(int* a, int* b);
void draw_line(int x0, int y0, int x1, int y1, short int color);
void draw_box(int x, int y, short int color);

void plot_pixel(int x, int y, short int line_color) {
  volatile short int* one_pixel_address;

  one_pixel_address = pixel_buffer_start + (y << 10) + (x << 1);

  *one_pixel_address = line_color;
}

void wait_for_vsync() {
  volatile int* pixel_ctrl_ptr = (int*)0xFF203020;
  *pixel_ctrl_ptr = 1;  // start v sync
  while ((*(pixel_ctrl_ptr + 3) & 0x1) != 0);
}

void clear_screen() {
  for (int i = 0; i < 240; i++) {
    for (int j = 0; j < 320; j++) {
      plot_pixel(j, i, 0x0);
    }
  }
}

#define TERMS 10

// Function to calculate power of x^n
double power(double x, int n) {
  double result = 1.0;
  for (int i = 0; i < n; i++) {
    result *= x;
  }
  return result;
}

// Function to calculate factorial of n
long long factorial(int n) {
  long long result = 1;
  for (int i = 2; i <= n; i++) {
    result *= i;
  }
  return result;
}

// Function to compute sin(x) using Taylor series
double sin_taylor(double x) {
  double sum = 0.0;
  for (int n = 0; n < TERMS; n++) {
    int exponent = 2 * n + 1;
    double term = power(x, exponent) / factorial(exponent);
    if (n % 2 == 1) {
      term = -term;
    }
    sum += term;
  }
  return sum;
}

// Function to compute cos(x) using Taylor series
double cos_taylor(double x) {
  double sum = 0.0;
  for (int n = 0; n < TERMS; n++) {
    int exponent = 2 * n;
    double term = power(x, exponent) / factorial(exponent);
    if (n % 2 == 1) {
      term = -term;
    }
    sum += term;
  }
  return sum;
}

void swap(int* a, int* b) {
  int temp = *b;
  *b = *a;
  *a = temp;
}

void draw_box(int x, int y, short int color){
    plot_pixel(x, y, color);
    plot_pixel(x, y - 1, color);
    plot_pixel(x, y + 1, color);
    plot_pixel(x + 1, y + 1, color);
    plot_pixel(x + 1, y, color);
    plot_pixel(x + 1, y - 1, color);
    plot_pixel(x - 1, y + 1, color);
    plot_pixel(x - 1, y, color);
    plot_pixel(x - 1, y - 1, color);
    plot_pixel(x + 2, y + 1, color);
    plot_pixel(x + 2, y, color);
    plot_pixel(x + 2, y - 1, color);
    plot_pixel(x - 2, y + 1, color);
    plot_pixel(x - 2, y, color);
    plot_pixel(x - 2, y - 1, color);
}

void draw_line(int x0, int y0, int x1, int y1, short int color) {
  // 1.determine which axis is steeper
  // 2. if y steep, work along y, if x is steep, walk along x
  //(y - y0) = m(x - x0) -> x are the i value;
  int unsigned_delta_y = y1 - y0;
  int unsigned_delta_x = x1 - x0;

  if (unsigned_delta_x < 0) {
    unsigned_delta_x *= -1;
  }
  if (unsigned_delta_y < 0) {
    unsigned_delta_y *= -1;
  }

  if (unsigned_delta_x < unsigned_delta_y) {
    swap(&x0, &y0);
    swap(&x1, &y1);
  }
  if (x0 > x1) {
    swap(&x0, &x1);
    swap(&y0, &y1);
  }

  int delta_x = x1 - x0;
  int delta_y = y1 - y0;
  if (delta_y < 0) {
    delta_y *= -1;
  }
  int error = -(delta_x / 2);
  int y = y0;
  int y_step = 0;
  if (y0 < y1) {
    y_step = 1;
  } else {
    y_step = -1;
  }

  for (int i = x0; i <= x1; i++) {
    if (unsigned_delta_x < unsigned_delta_y) {
      draw_box(y, i, color);
    } else {
      draw_box(i, y, color);
    }
    error = error + delta_y;
    if (error > 0) {
      y = y + y_step;
      error = error - delta_x;
    }
  }
}

void draw_LiDAR_frame(){
  for (int i = 0; i <= 180; i++){
    int x1 = 155 * cos_taylor(i/180.0 * PI) + 160;
    int y1 = 229 - 155 * sin_taylor(i/180.0 * PI);

    int x2 = 75 * cos_taylor(i/180.0 * PI) + 160;
    int y2 = 229 - 75 * sin_taylor(i/180.0 * PI);

    draw_box(x1, y1, 0x07E0);
    draw_box(x2, y2, 0x07E0);

    if(i == 90 || i == 45 || i == 0 || i == 180 || i == 135){
      draw_line(160, 229, x1, y1, 0x07E0);
    }
  }
  
}

#define LETTER_WIDTH   20
#define LETTER_HEIGHT  50
#define LETTER_GAP      5
#define BOLD_THICKNESS  2

// Helper: draws a block of pixels (of size thickness x thickness) for a bold effect.
void plot_bold_pixel(int x, int y, short int line_color, int thickness) {
    for (int dx = 0; dx < thickness; dx++) {
        for (int dy = 0; dy < thickness; dy++) {
            plot_pixel(x + dx, y + dy, line_color);
        }
    }
}

// Draws a single letter (in a 20x50 region) using a bold style.
void draw_letter(char c, int offset_x, int offset_y) {
    int row, col;
    switch(c) {
        case 'L':
            // Bold vertical stroke on left (2 pixels wide)
            for (row = 0; row < LETTER_HEIGHT; row++) {
                for (col = 0; col < BOLD_THICKNESS; col++) {
                    plot_bold_pixel(offset_x + col, offset_y + row, 0xFFFF, BOLD_THICKNESS);
                }
            }
            // Bold horizontal stroke at bottom.
            for (col = 0; col < LETTER_WIDTH; col++) {
                for (row = 0; row < BOLD_THICKNESS; row++) {
                    plot_bold_pixel(offset_x + col, offset_y + LETTER_HEIGHT - 1 - row, 0xFFFF, BOLD_THICKNESS);
                }
            }
            break;
            
        case 'i':
            {
                int mid = LETTER_WIDTH / 2;
                // Bold dot: draw a small filled square at top center.
                for (row = 0; row < BOLD_THICKNESS * 2; row++) {
                    for (col = 0; col < BOLD_THICKNESS * 2; col++) {
                        plot_bold_pixel(offset_x + mid + col, offset_y + 2 + row, 0xFFFF, BOLD_THICKNESS);
                    }
                }
                // Bold vertical stroke: now starting at row 9 (shifted down by 3 pixels) 
                // and ending at LETTER_HEIGHT - 10.
                for (row = 12; row < LETTER_HEIGHT; row++) {
                    for (col = 0; col < BOLD_THICKNESS; col++) {
                        plot_bold_pixel(offset_x + mid + col, offset_y + row, 0xFFFF, BOLD_THICKNESS);
                    }
                }
            }
            break;
            
        case 'D':
            {
                // Bold left vertical stroke.
                for (row = 0; row < LETTER_HEIGHT; row++) {
                    for (col = 0; col < BOLD_THICKNESS; col++) {
                        plot_bold_pixel(offset_x + col, offset_y + row, 0xFFFF, BOLD_THICKNESS);
                    }
                }
                // Bold top horizontal stroke.
                for (col = 0; col < LETTER_WIDTH - 5; col++) {
                    for (row = 0; row < BOLD_THICKNESS; row++) {
                        plot_bold_pixel(offset_x + col, offset_y + row, 0xFFFF, BOLD_THICKNESS);
                    }
                }
                // Bold bottom horizontal stroke.
                for (col = 0; col < LETTER_WIDTH - 5; col++) {
                    for (row = 0; row < BOLD_THICKNESS; row++) {
                        plot_bold_pixel(offset_x + col, offset_y + LETTER_HEIGHT - 1 - row, 0xFFFF, BOLD_THICKNESS);
                    }
                }
                // Bold right vertical stroke (simulate the curved side) for the middle area.
                for (row = 5; row < LETTER_HEIGHT - 5; row++) {
                    for (col = 0; col < BOLD_THICKNESS; col++) {
                        plot_bold_pixel(offset_x + LETTER_WIDTH - 1 - col, offset_y + row, 0xFFFF, BOLD_THICKNESS);
                    }
                }
            }
            break;
            
        case 'A':
            {
                int mid = LETTER_WIDTH / 2;
                // Bold left diagonal stroke.
                for (row = 0; row < LETTER_HEIGHT; row++) {
                    int col_left = (mid * row) / (LETTER_HEIGHT - 1);
                    for (int t = 0; t < BOLD_THICKNESS; t++) {
                        plot_bold_pixel(offset_x + col_left + t, offset_y + LETTER_HEIGHT - 1 - row, 0xFFFF, BOLD_THICKNESS);
                    }
                    // Bold right diagonal stroke.
                    int col_right = LETTER_WIDTH - 1 - ((mid * row) / (LETTER_HEIGHT - 1));
                    for (int t = 0; t < BOLD_THICKNESS; t++) {
                        plot_bold_pixel(offset_x + col_right - t, offset_y + LETTER_HEIGHT - 1 - row, 0xFFFF, BOLD_THICKNESS);
                    }
                }
                // Bold horizontal bar in the middle.
                int bar_y = offset_y + LETTER_HEIGHT / 2;
                for (col = 5; col < LETTER_WIDTH - 5; col++) {
                    for (row = 0; row < BOLD_THICKNESS; row++) {
                        plot_bold_pixel(offset_x + col, bar_y + row, 0xFFFF, BOLD_THICKNESS);
                    }
                }
            }
            break;
            
        case 'R':
            {
                // Bold left vertical stroke.
                for (row = 0; row < LETTER_HEIGHT; row++) {
                    for (col = 0; col < BOLD_THICKNESS; col++) {
                        plot_bold_pixel(offset_x + col, offset_y + row, 0xFFFF, BOLD_THICKNESS);
                    }
                }
                // Bold top horizontal stroke.
                for (col = 0; col < LETTER_WIDTH - 5; col++) {
                    for (row = 0; row < BOLD_THICKNESS; row++) {
                        plot_bold_pixel(offset_x + col, offset_y + row, 0xFFFF, BOLD_THICKNESS);
                    }
                }
                // Bold top right vertical stroke: now shifted down by 3 pixels.
                for (row = 0; row < 20; row++) {
                    for (col = 0; col < BOLD_THICKNESS; col++) {
                        // Adding 3 to the y coordinate shifts the stroke down.
                        plot_bold_pixel(offset_x + LETTER_WIDTH - 1 - col, offset_y + row + 3, 0xFFFF, BOLD_THICKNESS);
                    }
                }
                // Bold middle horizontal stroke.
                int mid_y = offset_y + LETTER_HEIGHT / 2;
                for (col = 0; col < LETTER_WIDTH - 5; col++) {
                    for (row = 0; row < BOLD_THICKNESS; row++) {
                        plot_bold_pixel(offset_x + col, mid_y + row, 0xFFFF, BOLD_THICKNESS);
                    }
                }
                // Bold diagonal leg from the middle to bottom right.
                for (row = LETTER_HEIGHT / 2; row < LETTER_HEIGHT; row++) {
                    int diag = (row - LETTER_HEIGHT / 2) * LETTER_WIDTH / (LETTER_HEIGHT / 2);
                    for (int t = 0; t < BOLD_THICKNESS; t++) {
                        plot_bold_pixel(offset_x + diag + t, offset_y + row, 0xFFFF, BOLD_THICKNESS);
                    }
                }
            }
            break;
            
        case 'T':
        case 't':
            {
                // Bold top horizontal stroke.
                for (col = 0; col < LETTER_WIDTH; col++) {
                    for (row = 0; row < BOLD_THICKNESS; row++) {
                        plot_bold_pixel(offset_x + col, offset_y + row, 0xFFFF, BOLD_THICKNESS);
                    }
                }
                // Bold vertical stroke in the center.
                int mid = LETTER_WIDTH / 2;
                for (row = 0; row < LETTER_HEIGHT; row++) {
                    for (col = 0; col < BOLD_THICKNESS; col++) {
                        plot_bold_pixel(offset_x + mid + col, offset_y + row, 0xFFFF, BOLD_THICKNESS);
                    }
                }
            }
            break;
            
        case 'E':
        case 'e':
            {
                // Bold left vertical stroke.
                for (row = 0; row < LETTER_HEIGHT; row++) {
                    for (col = 0; col < BOLD_THICKNESS; col++) {
                        plot_bold_pixel(offset_x + col, offset_y + row, 0xFFFF, BOLD_THICKNESS);
                    }
                }
                // Bold top horizontal stroke.
                for (col = 0; col < LETTER_WIDTH; col++) {
                    for (row = 0; row < BOLD_THICKNESS; row++) {
                        plot_bold_pixel(offset_x + col, offset_y + row, 0xFFFF, BOLD_THICKNESS);
                    }
                }
                // Bold middle horizontal stroke.
                int mid_y = offset_y + LETTER_HEIGHT / 2;
                for (col = 0; col < LETTER_WIDTH - 5; col++) {
                    for (row = 0; row < BOLD_THICKNESS; row++) {
                        plot_bold_pixel(offset_x + col, mid_y + row, 0xFFFF, BOLD_THICKNESS);
                    }
                }
                // Bold bottom horizontal stroke.
                for (col = 0; col < LETTER_WIDTH; col++) {
                    for (row = 0; row < BOLD_THICKNESS; row++) {
                        plot_bold_pixel(offset_x + col, offset_y + LETTER_HEIGHT - 1 - row, 0xFFFF, BOLD_THICKNESS);
                    }
                }
            }
            break;
            
        default:
            // For unsupported characters, draw a filled bold rectangle.
            for (row = 0; row < LETTER_HEIGHT; row++) {
                for (col = 0; col < LETTER_WIDTH; col++) {
                    plot_bold_pixel(offset_x + col, offset_y + row, 0xFFFF, BOLD_THICKNESS);
                }
            }
            break;
    }
}

// Plots the title "LiDAR Lite" in the center of a 320x240 VGA display.
// Each non-space letter is drawn in its own 20x50 box (centered vertically in the y=0..70 range)
// with a 5-pixel gap between letters.
void plot_title() {
    const char *title = "LiDAR Lite";  // 10 characters (skip spaces)
    const int num_chars = 10;
    
    // Total text block width:
    int text_width = num_chars * LETTER_WIDTH + (num_chars - 1) * LETTER_GAP; // 245 pixels
    int start_x = (320 - text_width) / 2;  // horizontally centered
    // Vertically center the 50-pixel-tall letter in the 0..70 region.
    int start_y = (70 - LETTER_HEIGHT) / 2;  // equals 10

    for (int i = 0; i < num_chars; i++) {
        int letter_x = start_x + i * (LETTER_WIDTH + LETTER_GAP);
        char c = title[i];
        if (c == ' ')
            continue;  // Skip drawing for a space.
        draw_letter(c, letter_x, start_y);
    }
}



// angle = pulse width -50

int calculate_x(distance, angle) {
  // r=150; max 2 meters
  int x;
  distance *= 2;
  if (distance >= 200) {
    distance = 200;
  }
  x = (int)distance * 0.75 * cos_taylor(angle/180.0 * PI) + 160;
  return x;
}

int calculate_y(distance, angle) {
  int y;
  distance *= 2;
  if (distance >= 200) {
    distance = 200;
  }
  y = (int)229 - distance * 0.75 * sin_taylor(angle/180.0 * PI);
  return y;
}

volatile unsigned int distance;
unsigned char signArray[] = {SEG_0, SEG_1, SEG_2, SEG_3, SEG_4,
                             SEG_5, SEG_6, SEG_7, SEG_8, SEG_9};

void ultrasonic_init() {
  // Set up JP1 Trig bit as output, Echo bit as input
  volatile int* parallel_port1_base = (int*)JP1_BASE;
  *(parallel_port1_base + 1) = 0x1;
}

void ultrasonic_CountDown_start() {
  volatile int* timer_base = (int*)TIMER1_BASE;
  // Fill up timer content
  *(timer_base + 2) = 0xffff;
  *(timer_base + 3) = 0xffff;
  *(timer_base) = 0x0;         // clear TO
  *(timer_base + 1) = 0b0110;  // set to continuous count mode. Disable
                               // interrupt
}

void ultrasonic_count_10us() {
  volatile int* timer_base = (int*)TIMER1_BASE;
  int timer_content = 1000;
  *(timer_base + 2) = timer_content;
  *(timer_base + 3) = 0x0;
  *(timer_base) = 0x0;         // clear TO
  *(timer_base + 1) = 0b0110;  // set to continuous count mode. Disable
                               // interrupt
}

void ultrasonic_timer_stop() {
  volatile int* timer_base = (int*)TIMER1_BASE;
  *(timer_base + 1) = 0b1010;
}

void ultrasonic_send_wave() {
  volatile int* parallel_port1_base = (int*)JP1_BASE;
  volatile int* timer_base = (int*)TIMER1_BASE;
  *(parallel_port1_base) |= (1 << Trigger);
  ultrasonic_count_10us();
  while ((*timer_base & 0x1) == 0);
  ultrasonic_timer_stop();
  *(parallel_port1_base) &= ~(1 << Trigger);
}

int ultrasonic_compute_distance_cm() {
  volatile int* parallel_port1_base = (int*)JP1_BASE;
  volatile int* timer_base = (int*)TIMER1_BASE;
 // volatile int* store = 0x20000;
  double distance = 0;
  while (((*parallel_port1_base & (1 << Echo)) >> Echo) ==
         0);  // wait when echo is not received;
  ultrasonic_CountDown_start();
  while ((*parallel_port1_base & (1 << Echo)) >>
         Echo);  // wait when echo is high
  *(timer_base + 4) = 0x1;
  unsigned int timer_current_low = *(timer_base + 4);
  unsigned int timer_current_high = *(timer_base + 5);
  ultrasonic_timer_stop();
  unsigned int timer_current = timer_current_low + (timer_current_high << 16);
  distance = ((0xFFFFFFFF - timer_current) / 100000000.0) * (170) * 100.0;
  // Distance in cm
  return (int)distance;
  // round to 1 decimal places
}

#define TIMER2_BASE 0xFF202020
#define LED_base 0xFF200000
#define TIMER2_IRQ 17
#define TS 2000
#define Servo 2  // using DP2

#ifndef JP1_BASE
#define JP1_BASE 0xFF200060
#endif

#ifndef KEY_BASE
#define KEY_BASE 0xFF200050
#define KEY_IRQ 18
#endif


volatile int count = 0;
volatile int count_rotate = 0;
volatile int fbi = 1;
// int count2 = 0;
// double pwm_duty_ratio = 0.5; //default 50% duty ratio
volatile int* key_base = (int*)KEY_BASE;
volatile int pulse_width = 50;

void timer2_start();
void timer2_stop();
void timer2_ISR();
void __attribute__((interrupt("machine"))) ISR_HANDLER(void);
void timer2_interrupt_init();
void timer2_init();

void timer2_start() {
  int* timer2_base = (int*)TIMER2_BASE;
  *timer2_base = 0x0;
  *(timer2_base + 1) = 0b0111;
}

void timer2_stop() {
  int* timer2_base = (int*)TIMER2_BASE;
  *timer2_base = 0x0;
  *(timer2_base + 1) = 0b1010;
}

void timer2_ISR() {
  timer2_stop();
  volatile int* jp1 = (int*)JP1_BASE;
  // PWM generator
  count++;
  count %= TS;
  if (count < pulse_width) {
    // set corresponding pin to 1
    *jp1 |= (1 << Servo);
  } else {
    // set corresponding pin to 0
    *jp1 &= ~(1 << Servo);
  }
  timer2_start();
}

void __attribute__((interrupt("machine"))) ISR_HANDLER(
    void)  // declare this subroutine as an ISR
{
  int mcause_value = 0x0;
  __asm__ volatile("csrr %0, mcause" : "=r"(mcause_value));
  if (mcause_value == (TIMER2_IRQ + 0x80000000)) {
    timer2_ISR();
  }
}

void timer2_interrupt_init() {
  // THIS FUNCTION ONLY OPERATES THE GENERAL INTERRUPT CONTROL REGs
  volatile int* timer2_base = (int*)TIMER2_BASE;
  volatile int timer2_IRQ = TIMER2_IRQ;
  int mstatus_value, IRQ_value, mtvec_value;
  mstatus_value = 0b1000;  // disable global interrupt
  __asm__ volatile("csrc mstatus, %0" ::"r"(mstatus_value));

  IRQ_value |= (1 << timer2_IRQ);  // set timer2 IRQ to mie
  __asm__ volatile("csrs mie, %0" ::"r"(IRQ_value));

  mtvec_value = (int)&ISR_HANDLER;  // load mtvec
  __asm__ volatile("csrw mtvec, %0" ::"r"(mtvec_value));

  // enable global interrupt
  __asm__ volatile("csrs mstatus, %0" ::"r"(mstatus_value));
}

void timer2_init() {
  int* timer2_base = (int*)TIMER2_BASE;
  int timer2_content = 1000;  // 10 us
  *(timer2_base) = 0x0;
  *(timer2_base + 1) = 0b1011;
  *(timer2_base + 2) = timer2_content;
  *(timer2_base + 3) = timer2_content >> 16;
  timer2_interrupt_init();
}

#include <stdint.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

// Audio data: 8000Hz, 32-bit signed, mono
int32_t audio_samples[] = {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00008000, 0x00000000, 0xffff8000, 0xffff8000, 0xffff8000, 0xffff8000, 0x00000000, 0xffff8000, 0xffff8000, 0x00000000, 0x00000000, 0x00000000, 0xffff8000, 0xffff8000, 0xffff0000, 0xffff0000, 0xfffe8000, 0xfffe8000, 0xffff0000, 0x00008000, 0x00008000, 0x00010000, 0xffff0000, 0xfffd8000, 0xfffd0000, 0xfffe0000, 0xfffd8000, 0xfffd0000, 0xfffd0000, 0xfffd0000, 0xfffd8000, 0xfffd0000, 0xfffc8000, 0xfffe8000, 0xfffe8000, 0xfffc8000, 0xfffd0000, 0xffff0000, 0xffff0000, 0x00028000, 0x00040000, 0x00048000, 0x00000000, 0x00018000, 0x00038000, 0xffff0000, 0xfffe0000, 0x00000000, 0x00050000, 0x00088000, 0x000b0000, 0x000c0000, 0x000e0000, 0x00060000, 0x00050000, 0x00008000, 0xfffe8000, 0xfffc0000, 0xfff28000, 0xfff20000, 0xffed0000, 0xffe60000, 0xffe10000, 0xffe18000, 0xffe18000, 0xffe10000, 0xffe30000, 0xffdd8000, 0xffdc8000, 0xffd98000, 0xffd38000, 0xffd30000, 0xffd38000, 0xffdd0000, 0xffe20000, 0xffe58000, 0xffeb8000, 0xffe78000, 0xffeb0000, 0xffe78000, 0xfff00000, 0xffeb8000, 0xfff40000, 0xfff88000, 0x00058000, 0xfffb8000, 0x00038000, 0xffff0000, 0xfff18000, 0x000b0000, 0x000a8000, 0x00068000, 0xfff70000, 0xfffd0000, 0xfffe0000, 0x00088000, 0x000a8000, 0x00090000, 0x000d8000, 0x000c0000, 0x00028000, 0xfff48000, 0xfff28000, 0x00060000, 0x00090000, 0xfff58000, 0x000d0000, 0x00150000, 0x00028000, 0xfff58000, 0xffef0000, 0xfff08000, 0xfff88000, 0x00060000, 0x00098000, 0x000e8000, 0x00158000, 0x00110000, 0x00150000, 0x001e8000, 0x001c8000, 0x00040000, 0xfffe8000, 0xfffb8000, 0xfff98000, 0x00080000, 0x00120000, 0x00108000, 0xfffc0000, 0x001c8000, 0x00148000, 0xffff0000, 0xfffa0000, 0x000a8000, 0x000d0000, 0x000f8000, 0x001a8000, 0x002f8000, 0x00278000, 0x002e8000, 0x00130000, 0x00048000, 0x00220000, 0x002c8000, 0x002e8000, 0x00270000, 0x00308000, 0x002f0000, 0x00398000, 0x00488000, 0x00540000, 0x003d8000, 0x00468000, 0x003a8000, 0x003e8000, 0x00330000, 0x003e0000, 0x00428000, 0x004e8000, 0x00468000, 0x00338000, 0x00348000, 0x00080000, 0xfffe0000, 0x00018000, 0xfffa8000, 0xfffa0000, 0xfff60000, 0xfffb0000, 0xfff28000, 0xffde0000, 0xffcb8000, 0xffd18000, 0xffbc0000, 0xffc10000, 0xffbd0000, 0xffca8000, 0xffd30000, 0xffd40000, 0xffce0000, 0xffe88000, 0xfff90000, 0x00050000, 0x00128000, 0x00260000, 0x00280000, 0x00130000, 0x00048000, 0xffff8000, 0x001c8000, 0x000c8000, 0x00038000, 0x00210000, 0x003a0000, 0x00250000, 0x00250000, 0x003c8000, 0x00330000, 0x00308000, 0x00358000, 0x00328000, 0x00380000, 0x002a0000, 0x00310000, 0x003e8000, 0x004c8000, 0x00410000, 0x004e0000, 0x002d0000, 0x001e8000, 0x001a8000, 0x00330000, 0x005a0000, 0x00568000, 0x00630000, 0x00640000, 0x00700000, 0x007f8000, 0x008c8000, 0x00950000, 0x00760000, 0x00600000, 0x00558000, 0x00428000, 0x00448000, 0x00430000, 0x00380000, 0x00290000, 0x00308000, 0x00258000, 0x00208000, 0x00200000, 0x00078000, 0x00028000, 0x00100000, 0x003a0000, 0x00420000, 0x00388000, 0x00140000, 0x00260000, 0x00248000, 0x00250000, 0x002b8000, 0x003d0000, 0x00318000, 0x002d8000, 0x00490000, 0x00508000, 0x00398000, 0x004b0000, 0x00650000, 0x00610000, 0x00708000, 0x00620000, 0x00740000, 0x00810000, 0x008d0000, 0x008b0000, 0x00818000, 0x008e8000, 0x00820000, 0x006c0000, 0x00840000, 0x00790000, 0x00678000, 0x005c8000, 0x00678000, 0x00770000, 0x00648000, 0x005f0000, 0x005a8000, 0x00790000, 0x008b0000, 0x00808000, 0x007b0000, 0x00800000, 0x00508000, 0x00468000, 0x00558000, 0x00488000, 0x00428000, 0x004a8000, 0x00470000, 0x00390000, 0x003b8000, 0x00278000, 0x003c8000, 0x002f0000, 0x003c8000, 0x00298000, 0x00300000, 0x00230000, 0x002b8000, 0x00400000, 0x00398000, 0x00370000, 0x000c0000, 0x000f0000, 0x000e0000, 0xfffb8000, 0xfffc0000, 0xffff0000, 0x000c8000, 0x00110000, 0x001a0000, 0x00310000, 0x00350000, 0x00520000, 0x00508000, 0x003f0000, 0x00478000, 0x00508000, 0x00548000, 0x00458000, 0x00438000, 0x00370000, 0x003d0000, 0x004a8000, 0x00600000, 0x00588000, 0x004f8000, 0x00608000, 0x00628000, 0x00538000, 0x006a8000, 0x00708000, 0x007e0000, 0x00778000, 0x00718000, 0x007a0000, 0x00610000, 0x00658000, 0x00708000, 0x00628000, 0x00608000, 0x00648000, 0x00520000, 0x00508000, 0x004f0000, 0x00438000, 0x00420000, 0x00570000, 0x00510000, 0x00410000, 0x003c0000, 0x00310000, 0x002b0000, 0x002f8000, 0x00478000, 0x00598000, 0x00570000, 0x00598000, 0x005f0000, 0x00570000, 0x00578000, 0x004b8000, 0x004c0000, 0x005d0000, 0x00618000, 0x00560000, 0x007d8000, 0x00908000, 0x007c8000, 0x008a8000, 0x00990000, 0x00880000, 0x00850000, 0x00838000, 0x00680000, 0x006b0000, 0x005c8000, 0x006b0000, 0x00700000, 0x00658000, 0x005f0000, 0x005f0000, 0x00498000, 0x00420000, 0x00448000, 0x00408000, 0x00628000, 0x00650000, 0x00758000, 0x00790000, 0x00690000, 0x006d8000, 0x006f8000, 0x00838000, 0x00700000, 0x00668000, 0x004b8000, 0x00360000, 0x00248000, 0x001e8000, 0x00148000, 0x001a8000, 0x000f0000, 0x00198000, 0x00200000, 0x00168000, 0x00100000, 0x00000000, 0xfff08000, 0x00010000, 0x00098000, 0x00068000, 0xffe20000, 0xffd50000, 0xffd98000, 0xffd10000, 0xffd38000, 0xffdd8000, 0xffcc0000, 0xffd48000, 0xffd80000, 0xffcb8000, 0xffce0000, 0xffde8000, 0xfff20000, 0xffec8000, 0x00038000, 0xfffa0000, 0xfff60000, 0x00018000, 0x00120000, 0x000c8000, 0x00130000, 0x000b8000, 0x00010000, 0xfff30000, 0x000a8000, 0x00010000, 0x000d0000, 0x00040000, 0x00118000, 0x00280000, 0x00238000, 0x00228000, 0x00108000, 0x00318000, 0x00328000, 0x00288000, 0x003e0000, 0x003e0000, 0x000f8000, 0xfffe0000, 0x00100000, 0xfffe0000, 0x000e8000, 0x00240000, 0x00298000, 0x00160000, 0x00258000, 0x001b0000, 0x00260000, 0x00020000, 0x002b0000, 0x00218000, 0x00328000, 0x00498000, 0x001f8000, 0x002e0000, 0x00060000, 0x00308000, 0x00040000, 0x001f0000, 0xffee0000, 0x00120000, 0xffed8000, 0x00490000, 0x00198000, 0x004a8000, 0x00388000, 0x007a0000, 0xfff48000, 0x00920000, 0x004a8000, 0x011a0000, 0x00ab0000, 0x009e0000, 0x00be0000, 0xff608000, 0x006a0000, 0x00000000, 0x004e0000, 0xfea48000, 0xffd10000, 0x00188000, 0x00000000, 0xfe988000, 0x001b0000, 0x001c0000, 0xfe368000, 0xff820000, 0xffc20000, 0x00ff8000, 0xff808000, 0xffca0000, 0x00a68000, 0x013b0000, 0xff5c8000, 0x00a90000, 0x017f0000, 0x011b0000, 0xff3d0000, 0x013e0000, 0x018f0000, 0x00750000, 0xffa10000, 0xfee90000, 0x00b00000, 0xff730000, 0xff9a0000, 0x01580000, 0x007c8000, 0xfefc0000, 0xff6f0000, 0xff058000, 0xfff58000, 0xfec18000, 0xff278000, 0x016c0000, 0xffb58000, 0x00828000, 0xfe678000, 0x009d8000, 0x00fd8000, 0xfe170000, 0x00968000, 0x01ed0000, 0x01e30000, 0x01b88000, 0x022e0000, 0x02828000, 0x02ef0000, 0xff068000, 0x007a8000, 0xfffc8000, 0x003c0000, 0x00478000, 0xfff28000, 0xfea30000, 0xfdd68000, 0xfc918000, 0xffa00000, 0xff990000, 0xfde60000, 0xfede8000, 0xfe388000, 0x00bc8000, 0x00998000, 0x01958000, 0x01a50000, 0xffb90000, 0x03430000, 0x04578000, 0x02398000, 0x00f70000, 0x02838000, 0x00b20000, 0x01d28000, 0x01a90000, 0x004c8000, 0x01220000, 0x03758000, 0xfd460000, 0x00298000, 0x01f98000, 0xfeb90000, 0x00d60000, 0xfe490000, 0xfecf8000, 0x02860000, 0xfeb70000, 0x0a340000, 0x58f60000, 0x58948000, 0x55490000, 0x58af8000, 0x56df0000, 0x58cc8000, 0x599e0000, 0x557c8000, 0x5a4b0000, 0x2db90000, 0xab310000, 0xa8b80000, 0xabbf0000, 0xaa338000, 0xacb00000, 0xaa158000, 0xa9790000, 0xaa080000, 0xa3ae0000, 0xe0b70000, 0x54d90000, 0x554d0000, 0x53a60000, 0x567a0000, 0x53a10000, 0x58dc0000, 0x55b20000, 0x57190000, 0x5ef98000, 0x1af30000, 0xa7fe0000, 0xabd60000, 0xafc60000, 0xaa760000, 0xadb80000, 0xaee00000, 0xaa780000, 0xa5d20000, 0xad510000, 0x45960000, 0x5c160000, 0x50470000, 0x50c90000, 0x56470000, 0x50400000, 0x582d0000, 0x550d0000, 0x58350000, 0x5de50000, 0x5cb30000, 0x577b0000, 0x55dd0000, 0x59ac0000, 0x5ba50000, 0x58e50000, 0x51990000, 0x56ac0000, 0x5ace8000, 0x5a570000, 0x53d80000, 0x56300000, 0x61d40000, 0x56650000, 0x58b20000, 0x534d0000, 0x545b0000, 0x595b0000, 0x50e18000, 0x56ac0000, 0x9d240000, 0xa8030000, 0xa79e8000, 0xa5d40000, 0xa5320000, 0xa6b80000, 0xa72d0000, 0xa3ad0000, 0xb38a0000, 0xaf700000, 0x65b58000, 0x57708000, 0x56ce0000, 0x58da8000, 0x53ac0000, 0x68980000, 0x63878000, 0x59e98000, 0x4e5e0000, 0x550c0000, 0xa6a10000, 0xab318000, 0xa0e90000, 0xafd30000, 0xa6680000, 0x9b018000, 0xa48b8000, 0xad300000, 0xaeaa0000, 0x9e320000, 0x57830000, 0x52fa0000, 0x51b40000, 0x4c838000, 0x52c40000, 0x589f0000, 0x565e8000, 0x5f4c8000, 0x54ab8000, 0x56e48000, 0x539a8000, 0x50638000, 0x5a878000, 0x5dd00000, 0x56f70000, 0x508c8000, 0x57310000, 0x51648000, 0x54618000, 0x58968000, 0x5b730000, 0x59340000, 0x54e30000, 0x592d0000, 0x5aac8000, 0x55cc8000, 0x53480000, 0x5c508000, 0x57388000, 0x5ac10000, 0xad258000, 0xac668000, 0xa6108000, 0xa6478000, 0xa7538000, 0xb0258000, 0xa8658000, 0xaa2f8000, 0xa1ec8000, 0xd4ae8000, 0x51ee0000, 0x56040000, 0x54ef0000, 0x56f58000, 0x5a268000, 0x51208000, 0x55c98000, 0x56260000, 0x57258000, 0x11948000, 0xa6be0000, 0x9df68000, 0xa8f78000, 0xaa698000, 0xa7118000, 0xad910000, 0xa5b58000, 0xa6a98000, 0xaab20000, 0xf8fb8000, 0x5b5d0000, 0x5c2a0000, 0x56c00000, 0x53810000, 0x5ba48000, 0x50f68000, 0x5b0f0000, 0x59850000, 0x45e08000, 0xb28d0000, 0xa2250000, 0xaa4f8000, 0xa6b70000, 0xa56e0000, 0xad0e0000, 0xa8090000, 0xaaee0000, 0xa7090000, 0xb37c8000, 0x4f428000, 0x60160000, 0x55908000, 0x598c0000, 0x5cbf8000, 0x55f30000, 0x56f10000, 0x5b890000, 0x55fe8000, 0x4db98000, 0xa6080000, 0xa10c0000, 0xa82e8000, 0xa7b00000, 0xa4b90000, 0xabd08000, 0xa8498000, 0xa5660000, 0xb2778000, 0xa7180000, 0x63258000, 0x4fd38000, 0x502a0000, 0x52cb0000, 0x56ec8000, 0x577a8000, 0x59520000, 0x56790000, 0x4c630000, 0x583c0000, 0xa09d8000, 0xb1370000, 0xb18d0000, 0xb23f0000, 0xb09d0000, 0xab740000, 0xaa180000, 0xa9920000, 0xae680000, 0xa5a38000, 0x5fb08000, 0x48f88000, 0x53248000, 0x54aa0000, 0x4fe00000, 0x56eb0000, 0x591d8000, 0x53d28000, 0x57b90000, 0x66308000, 0xad1e0000, 0xafba8000, 0xad0f8000, 0xabf88000, 0xb1380000, 0xa7bc8000, 0xa7d00000, 0xafad0000, 0xa8558000, 0x9e268000, 0x4c6a0000, 0x54878000, 0x59dc8000, 0x56548000, 0x50478000, 0x4df30000, 0x57e90000, 0x58a08000, 0x593d8000, 0x59640000, 0xb1438000, 0xaa840000, 0xa3780000, 0xa5140000, 0xa4150000, 0xaa578000, 0xa3458000, 0xa4598000, 0x9d3e0000, 0xb05a0000, 0x4b3a0000, 0x50840000, 0x52a58000, 0x59518000, 0x5b670000, 0x59840000, 0x57768000, 0x59fa8000, 0x60d78000, 0x0f5a0000, 0xb2ca0000, 0xa6dd0000, 0xa77c0000, 0xa9f28000, 0xacf48000, 0xa8ff0000, 0xa7158000, 0xabb20000, 0x9fc78000, 0xe8920000, 0x57bd0000, 0x5d108000, 0x55e28000, 0x5b060000, 0x58180000, 0x59660000, 0x55df8000, 0x4f3a8000, 0x5b760000, 0x57d50000, 0x543e0000, 0x53900000, 0x54290000, 0x574c0000, 0x55ef0000, 0x505b8000, 0x59198000, 0x56a60000, 0x57278000, 0x5d240000, 0x5a890000, 0x5b870000, 0x56ca0000, 0x5bc08000, 0x52be8000, 0x58ba0000, 0x555d0000, 0x5d9b0000, 0x47fc0000, 0xa9228000, 0xa9180000, 0xaaa48000, 0xa4670000, 0xaa0c0000, 0xab560000, 0xacc90000, 0xa92a8000, 0xa8a58000, 0xb1798000, 0x0afb0000, 0x01968000, 0xfa338000, 0xfe830000, 0x03150000, 0x024f8000, 0x06048000, 0xfdb80000, 0xfcf88000, 0xf8f58000, 0xf9350000, 0xfa968000, 0x01ec8000, 0xff340000, 0x04140000, 0xfd3e8000, 0xf9880000, 0x03490000, 0xff5a0000, 0xff2f8000, 0xfde38000, 0xff6a8000, 0xfea70000, 0x015d0000, 0x03658000, 0xfa930000, 0xfced0000, 0xfdff0000, 0x01be0000, 0x02798000, 0xfc6d0000, 0xfc880000, 0xff3e0000, 0x00d90000, 0x01630000, 0x000a0000, 0x003c8000, 0x02ea0000, 0xfde60000, 0x03898000, 0x03d48000, 0x02c00000, 0xffc80000, 0xfeb30000, 0xf9890000, 0xfe850000, 0xfcab0000, 0xfcda0000, 0xfa4e8000, 0xfca70000, 0xfc380000, 0x00040000, 0x02788000, 0x02b80000, 0x01fd8000, 0xff480000, 0x01720000, 0xfec88000, 0xfe108000, 0xfb3e8000, 0xffdd0000, 0xfdf20000, 0xfae08000, 0xfa8a8000, 0xffe30000, 0x00168000, 0x00448000, 0xfd378000, 0x03f30000, 0x02488000, 0x00120000, 0x00528000, 0x00bf8000, 0xfdc38000, 0xfc370000, 0xfd7d8000, 0xfde50000, 0xfe930000, 0xfe618000, 0xff818000, 0x00208000, 0xffc98000, 0x00420000, 0x01180000, 0x00b58000, 0x01360000, 0xfe2e0000, 0x02ab8000, 0x017e8000, 0x00970000, 0x008a8000, 0x01ee8000, 0x03950000, 0x000a8000, 0xfef30000, 0xfbd68000, 0xfd888000, 0xff560000, 0xfd9d8000, 0xfee88000, 0xff4c0000, 0x001c8000, 0xfde28000, 0x013c0000, 0x01410000, 0xfeb88000, 0xfb8b0000, 0xfb9a0000, 0xfd7b0000, 0xfca20000, 0xfbd70000, 0xfe788000, 0xff2b0000, 0xff778000, 0xfcc00000, 0x00ff8000, 0x02b00000, 0x01f28000, 0xfffa8000, 0xfff50000, 0x017f8000, 0xfe898000, 0xfd998000, 0x00110000, 0xffc08000, 0xfa998000, 0xfa930000, 0xfc838000, 0x01498000, 0x00238000, 0xff570000, 0x03100000, 0x05370000, 0x03520000, 0x03328000, 0x03138000, 0x03968000, 0xffef8000, 0xff0b8000, 0xffb90000, 0x01430000, 0x01848000, 0x00c30000, 0x02918000, 0x03f88000, 0x022f8000, 0x01ea8000, 0x02558000, 0x02c10000, 0x00a18000, 0xfe078000, 0xff300000, 0x013f0000, 0xffe30000, 0x01f08000, 0x04b58000, 0x03748000, 0x04228000, 0x04178000, 0x04988000, 0x06148000, 0x05430000, 0x055d8000, 0x05930000, 0x03030000, 0x03f00000, 0x02430000, 0x065e0000, 0x04090000, 0x02388000, 0x01958000, 0x037d0000, 0x04a38000, 0x041f8000, 0x04ca8000, 0x05a48000, 0x03790000, 0x02628000, 0x03480000, 0x04cb0000, 0x02ea8000, 0x00f58000, 0x00b48000, 0x03c78000, 0x02778000, 0x00298000, 0x04268000, 0x03a98000, 0x02ba8000, 0x013b0000, 0x02e68000, 0x054c0000, 0x04b30000, 0x014a0000, 0x03648000, 0x043d8000, 0x04e40000, 0x02f60000, 0x05818000, 0x05140000, 0x03260000, 0x01388000, 0x03148000, 0x04560000, 0x03140000, 0x020e0000, 0x03af8000, 0x037e8000, 0x00a80000, 0x015a0000, 0x03218000, 0x02590000, 0xffd28000, 0xfed78000, 0xff3e8000, 0x00c20000, 0xfe9c0000, 0xfe8e8000, 0x01e98000, 0x040d0000, 0x02a08000, 0x02f70000, 0x02b18000, 0x02c48000, 0x01600000, 0x009c0000, 0x019d8000, 0x015c0000, 0xfee08000, 0xfeaf0000, 0xffe58000, 0x01f30000, 0x00668000, 0x023a0000, 0x04238000, 0x028f0000, 0xfee18000, 0xfdfa0000, 0x02270000, 0x00fe0000, 0xfec98000, 0xfe028000, 0x007c0000, 0x00bf0000, 0xfda88000, 0xfe908000, 0x013d8000, 0x006c8000, 0xfe220000, 0xfe4d8000, 0x01930000, 0x00258000, 0xfe8d8000, 0xfdb48000, 0x01350000, 0x015a8000, 0xff800000, 0x00818000, 0x02e20000, 0x00710000, 0xfd4e0000, 0xfdf30000, 0xffef8000, 0xffc40000, 0xfc3a8000, 0xfb8e8000, 0xfde48000, 0xfccb0000, 0xf9a20000, 0xfcdb8000, 0xff588000, 0xfe6f8000, 0xfce88000, 0xfdc70000, 0x01de0000, 0x00ba8000, 0xfe3b8000, 0xff738000, 0x011c8000, 0xfec48000, 0xfc348000, 0xfde60000, 0xfee90000, 0xfd2d8000, 0xf9ba0000, 0xfbc78000, 0xfdf50000, 0xfca20000, 0xfa198000, 0xfc210000, 0xff0b0000, 0xfd908000, 0xfd6f8000, 0xff3d0000, 0x01d60000, 0xffdd8000, 0xfee50000, 0x002e0000, 0x00860000, 0x00258000, 0xff620000, 0x000d8000, 0x00540000, 0xff9d0000, 0xfffc0000, 0x00258000, 0x001a0000, 0xff870000, 0xfffa8000, 0x00380000, 0x00298000, 0xffe38000, 0xfff00000, 0x00228000, 0x00280000, 0xffd98000, 0xfff28000, 0x00268000, 0x00020000, 0xffe50000, 0xffe68000, 0x00190000, 0xfff70000, 0xffec0000, 0xffeb0000, 0x00100000, 0x00060000, 0x00020000, 0xfffe8000, 0x000d8000, 0x00030000, 0x000d8000, 0xfffe8000, 0x00050000, 0xfff08000, 0xfff10000, 0xffec8000, 0xfffc0000, 0x00080000, 0xfffc8000, 0xfffd8000, 0x00078000, 0xfffc0000, 0xfffb0000, 0x00070000, 0x00028000, 0x00010000, 0x00048000, 0x00000000, 0xfff98000, 0xfffe0000, 0x00020000, 0x00048000, 0xfff98000, 0xfffe0000, 0x00080000, 0x00080000, 0xfffd0000, 0x00050000, 0x00058000, 0x00030000, 0xfffe8000, 0x00008000, 0x00068000, 0x00020000, 0xfffc0000, 0xffff8000, 0x00008000, 0xfffe0000, 0xfffa0000, 0xffff0000};
int num_samples = ARRAY_SIZE(audio_samples);

// Audio playback interface structure
struct audio_interface {
    volatile unsigned int control;
    volatile unsigned char read_available;
    volatile unsigned char read_left;
    volatile unsigned char write_available;
    volatile unsigned char write_left;
    volatile unsigned int left_data;
    volatile unsigned int right_data;
};

// Pointer to the audio interface hardware registers
struct audio_interface *const audio = (struct audio_interface *)0xff203040;

int voice_sample_counter = 0;
int dist_arr[20] = {50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50};

int main() {
  timer2_init();
  timer2_start();
  ultrasonic_init();
  volatile int* seg_base = (int*)HEX3_HEX0_BASE;
  volatile int* jp1Base = (int*)JP1_BASE;
  int angle_inc = 0;
  *(jp1Base + 1) |= (1 << Servo);

  volatile int* pixel_ctrl_ptr = (int*)0xFF203020;
  /* Read location of the pixel buffer from the pixel buffer controller */
  pixel_buffer_start = *pixel_ctrl_ptr;
  *(pixel_ctrl_ptr + 1) = pixel_buffer_start;  // setting the back buffer to be
                                               // the same as front buffer
  int angle = 0;
  
  int distance_x[181] = {0};
  int distance_y[181] = {0};

  for (int i = 0; i < 181; i ++){
    distance_x[i] = 160;
    distance_y[i] = 229;
  }

  double cos45 = cos_taylor(45/180.0 * PI);
  double sin45 = sin_taylor(45/180.0 * PI);

  clear_screen();
  plot_title();
  draw_LiDAR_frame();

  while (1) {
    unsigned int segDisplay = 0x0;
    int i = 0;
    int dist;
    
    ultrasonic_send_wave();
    distance = ultrasonic_compute_distance_cm();
    //distance = 50;
    dist = distance;
    volatile int* led_base = 0xFF200000;
    *led_base ^= 0x1;
    while (i != 32) {
      unsigned char lsd = distance % 10;
      segDisplay += (signArray[lsd] << i);
      i += 8;
      distance /= 10;
    }

    *seg_base = segDisplay;
    if (angle <= 0) {
      angle_inc = 1;
    } else if (angle >= 180) {
      angle_inc = -1;
    }
    angle += angle_inc;
    pulse_width = angle + 50;

    draw_line(160, 229, distance_x[angle], distance_y[angle], 0x0);
    distance_x[angle] = calculate_x(dist, angle);
    distance_y[angle] = calculate_y(dist, angle);
    draw_box(75 * cos_taylor((angle - angle_inc)/180.0 * PI) + 160, 229 - 75 * sin_taylor((angle - angle_inc)/180.0 * PI), 0x07E0);

    // short int color = boxColor[i];
    draw_line(160, 229, distance_x[angle], distance_y[angle], 0xf800);

    draw_box(75 * cos_taylor(angle/180.0 * PI) + 160, 229 - 75 * sin_taylor(angle/180.0 * PI), 0x07E0);
    
    draw_line(160, 229, 155 + 160, 229, 0x07E0);
    draw_line(160, 229, 155 * cos45 + 160, 229 - 155 * sin45, 0x07E0);
    draw_line(160, 229, 160, 229 - 155, 0x07E0);
    draw_line(160, 229, 155 * (-1) * cos45 + 160, 229 - 155 * sin45, 0x07E0);
    draw_line(160, 229, 155 * (-1) + 160, 229, 0x07E0);

    if(!angle || angle == 180){
      draw_LiDAR_frame();
    }
        
    voice_sample_counter++;
    voice_sample_counter %= 20;
    dist_arr[voice_sample_counter] = dist;
    if (voice_sample_counter==0){
    	audio->control = 0x8;
    	audio->control = 0x0;
    	int dist_A = 0;
      for (int j=0; j<20; j++){
    	  dist_A += dist_arr[j];
      }
    	dist_A /= 20;
      for (int i = 0; i < num_samples; i ++) {
        // Wait until there is space in the FIFO
        while (audio->write_available == 0);
        // Write the sample to both channels
        audio->left_data = audio_samples[i] / dist_A;
        audio->right_data = audio_samples[i] / dist_A;
      }
    
    }
        
    // code for updating the locations of boxes (not shown)
    wait_for_vsync();  // swap front and back buffers on VGA vertical sync
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);  // new back buffer
  }
}
