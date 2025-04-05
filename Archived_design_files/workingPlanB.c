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

    
    
    // code for updating the locations of boxes (not shown)
    wait_for_vsync();  // swap front and back buffers on VGA vertical sync
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);  // new back buffer
  }
}
