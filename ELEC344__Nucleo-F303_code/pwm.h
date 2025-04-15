#ifndef _PWM_H__
#define _PWM_H__


void init_pwm(unsigned int F_pwm);
void output_pwm(float d);

extern unsigned short PWM_MAX;
#endif
