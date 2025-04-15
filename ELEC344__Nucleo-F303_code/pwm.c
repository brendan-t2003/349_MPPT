#include <stm32f3xx.h>
#include "pwm.h"

unsigned short PWM_MAX;
void init_pwm(unsigned int F_pwm)
{
	#define AF_tim3	2u
	RCC->AHBENR|=RCC_AHBENR_GPIOBEN;
	GPIOB->MODER&=~GPIO_MODER_MODER0;
	GPIOB->MODER|=GPIO_MODER_MODER0_1;
	GPIOB->AFR[0]&=~GPIO_AFRL_AFRL0;
	GPIOB->AFR[0]|=(AF_tim3<<GPIO_AFRL_AFRL0_Pos);
	
	RCC->APB1ENR|=RCC_APB1ENR_TIM3EN;
	unsigned short psc3=1;
	unsigned short arr3=(unsigned short)(((SystemCoreClock/(psc3*F_pwm))));
	TIM3->CCMR2=(6u<<TIM_CCMR2_OC3M_Pos);
	TIM3->CCER|=TIM_CCER_CC3E;
	TIM3->PSC=psc3-1;
	TIM3->ARR=arr3;
	TIM3->CNT=0;
	TIM3->CR1|=TIM_CR1_CEN;
	
	PWM_MAX=arr3;
}

void output_pwm(float d)
{
	if(d>100.0f) d=100.0f;
	else if(d<0.0f) d=0.0f;
	TIM3->CCR3=(unsigned short)(d*(float)(PWM_MAX)/100.0f);
}
