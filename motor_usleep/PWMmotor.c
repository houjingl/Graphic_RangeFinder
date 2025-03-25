#ifndef JP1_BASE
#define JP1_BASE  0xFF200060
#define TIMER1_BASE  0xFF202000
#endif

#define Servo 2 // using the first one seen
                          //  in the document
#define PWM_PERIOD 20000  // 20ms period in microseconds

void pwm_servo(int angle) {
  volatile int* jp1 = (int*) JP1_BASE;
  int pulse_width =
      1000 + (angle * 1000 / 180);  // Convert angle to pulse width (1-2ms)
  *(jp1) |= (1<< Servo);
  usleep(pulse_width);                         // Wait for pulse width duration
  *(jp1) &= (0<< Servo);  // Set GPIO LOW
  usleep(PWM_PERIOD - pulse_width);            // Maintain 20ms period
}

void usleep (int i){
    volatile int* timer_base = (int*) TIMER1_BASE;
    int timer_content = 100*i;
    *(timer_base + 2) = timer_content;
    *(timer_base + 3) = 0x0;
    *(timer_base) = 0x0; //clear TO
    *(timer_base + 1) = 0b0110; //set to continuous count mode. Disable interrupt
}

int main(){
  while (1){
    pwm_servo(90);
    usleep(1000000);
    pwm_servo(0);
    usleep(1000000);    
  }
  return 0;
}