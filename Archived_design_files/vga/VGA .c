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

// #include <math.h>
// #include <stdio.h>
// #include <stdlib.h>

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
double sin(double x);
double cos(double x);

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

void swap(int* a, int* b) {
  int temp = *b;
  *b = *a;
  *a = temp;
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
      plot_pixel(y, i, color);
    } else {
      plot_pixel(i, y, color);
    }
    error = error + delta_y;
    if (error > 0) {
      y = y + y_step;
      error = error - delta_x;
    }
  }
}

// angle = pulse width -50

int calculate_x(distance, angle) {
  // r=150; max 2 meters
  int x;
  if (distance >= 200) {
    distance = 200;
  }
  x = (int)distance * 0.75 * cos_taylor(angle / 170.0 * PI) + 160;
  return x;
}

int calculate_y(distance, angle) {
  int y;
  if (distance >= 200) {
    distance = 200;
  }
  y = (int)229 - distance * 0.75 * sin_taylor(angle / 170.0 * PI);
  return y;
}

// int pixel_buffer_start; // global variable

int main(void) {
  volatile int* pixel_ctrl_ptr = (int*)0xFF203020;
  /* Read location of the pixel buffer from the pixel buffer controller */
  pixel_buffer_start = *pixel_ctrl_ptr;
  *(pixel_ctrl_ptr + 1) = pixel_buffer_start;  // setting the back buffer to be
                                               // the same as front buffer

  int distance_x[171] = {0};
  int distance_y[171] = {0};

  clear_screen();

  // int dist = 150;
  int angle = 0;
  int angle_inc = 0;
  int countForAngle = 0;
  while (1) {
    //    unsigned int segDisplay = 0x0;
    //    int i = 0;

    //    ultrasonic_send_wave();
    //    distance = ultrasonic_compute_distance_cm();
    //    dist = distance;

    volatile int* led_base = 0xFF200000;
    *led_base ^= 0x1;

    countForAngle++;
    countForAngle %= 3;
    if (countForAngle == 0) {
      if (angle <= 0) {
        angle_inc = 1;
      } else if (angle >= 170) {
        angle_inc = -1;
      }
      angle += angle_inc;
    }

    int dist = 150 + angle_inc * 50;

    //   while (i != 32) {
    //     unsigned char lsd = distance % 10;
    //     segDisplay += (signArray[lsd] << i);
    //     i += 8;
    //     distance /= 10;
    //   }
    //   *seg_base = segDisplay;

    //   angle = pulse_width - 50;
    // clear_screen();
    draw_line(160, 229, distance_x[angle], distance_y[angle], 0x0);

    if ((angle + angle_inc >= 0) && (angle + angle_inc <= 170)) {
      draw_line(160, 229, distance_x[angle + angle_inc],
                distance_y[angle + angle_inc], 0x0);
    }
    // short int color = boxColor[i];
    draw_line(160, 229, calculate_x(dist, angle), calculate_y(dist, angle),
              0xf800);
    distance_x[angle] = calculate_x(dist, angle);
    distance_y[angle] = calculate_y(dist, angle);
    // code for updating the locations of boxes (not shown)
    wait_for_vsync();  // swap front and back buffers on VGA vertical sync
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);  // new back buffer
  }
  return 0;
}
