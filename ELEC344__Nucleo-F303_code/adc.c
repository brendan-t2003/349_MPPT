#include <stm32f303x8.h>
#include "adc.h"

void ADC_Init(void)
{
				//GPIO CONFIG
	RCC->AHBENR|=RCC_AHBENR_GPIOAEN;	//GPIOA clock enabled
	GPIOA->MODER|=(GPIO_MODER_MODER0|GPIO_MODER_MODER1);
	
				//ADC CONFIG
	RCC->AHBENR|=RCC_AHBENR_ADC12EN;				//ADC clock enabled
	RCC->CFGR2|=0x11<<4;
	ADC1->CR&=~ADC_CR_ADVREGEN;
	ADC1->CR|=ADC_CR_ADVREGEN_0;	//set ADVREGEN bits to 01b
	ADC1->CR|=ADC_CR_ADEN;								//ADC on
	while(!(ADC1->ISR & ADC_ISR_ADRD));	//Wait until ADRDY=1 (ADRDY is set after the ADC startup time)
	
	

}



unsigned short read_adc(unsigned char channel)
{
	ADC1->SQR1=(unsigned int)(channel<<ADC_SQR1_SQ1_Pos);
	ADC1->CR|=ADC_CR_ADSTART;
	while(!(ADC1->ISR&ADC_ISR_EOC)){__NOP();}
	return (unsigned short)(ADC1->DR);
}


