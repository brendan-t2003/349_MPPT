#ifndef _ADC_H
#define _ADC_H

#define convert_PA0		read_adc(1)
#define convert_PA1		read_adc(2)

void ADC_Init(void);
unsigned short read_adc(unsigned char channel);

#endif
