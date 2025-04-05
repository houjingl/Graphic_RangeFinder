#ifndef __SERVO__PWM__
#define __SERVO__PWM__

#define servo_pwm_BASE          0xFF202020 //Timer 2
#define LED_base                0xFF200000
#define servo_pwm_IRQ           17
#define TS                      2000
#define Servo                   2            //using DP2

#ifndef JP1_BASE
#define JP1_BASE  0xFF200060
#endif

void servo_pwm_start();
void servo_pwm_stop();
void servo_pwm_ISR();
void servo_pwm_interrupt_init();
void servo_pwm_init();

#endif