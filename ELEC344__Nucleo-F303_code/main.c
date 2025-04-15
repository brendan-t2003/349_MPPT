#include <stm32f3xx.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adc.h"
#include "leds.h"
#include "usart.h"
#include "systick_delay.h"
#include "PLL_Config.h"
#include "pwm.h"

const int sample_count = 300;

float Vin_samples[300] = {0};
float Iin_samples[300] = {0};
int sample_index = 0;

float average(float *buffer, int size) {
    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        sum += buffer[i];
    }
    return sum / size;
}

int main(void)
{
    PLL_Config();
    SystemCoreClockUpdate();
    SysTick_Init();
    init_usart(115200);     // set baud rate
    ADC_Init();            
    init_led();             // LED on the bottom of board 
    init_pwm(100000);       // 100kHz

    float Vin = 0.0f, Iin = 0.0f, P = 0.0f;
    float avg_Vin = 0.0f, avg_Iin = 0.0f, avg_P = 0.0f;
    float prev_P = 0.0f, prev_Vin = 0.0f;
    float duty = 20.0f;     // try to start near expected MPP
    float step = 0.5f;      // base step size
    int direction = 1;      // 1 = increase -1 = decrease
    char term_msg[128];

    while (1)
    {
        delay_nms(2);
        led_on();

        // read ADC values
        uint16_t adc_V = convert_PA0;
        uint16_t adc_I = convert_PA1;

        // convert ADC to voltage and current to map them to panel
        Vin = (adc_V / 4095.0f) * 3.3f * 6.2f;
        Iin = (adc_I / 4095.0f) * 3.3f / 4.0f;

        // store samples
        Vin_samples[sample_index] = Vin;
        Iin_samples[sample_index] = Iin;
        sample_index++;

        // after enough samples are collected average them out
        if (sample_index >= sample_count) {
            sample_index = 0;

            avg_Vin = average(Vin_samples, sample_count);
            avg_Iin = average(Iin_samples, sample_count);
            avg_P = avg_Vin * avg_Iin;

            float delta_P = avg_P - prev_P;
            float delta_V = avg_Vin - prev_Vin;
            float step_multiplier = 1.0f; // step multiplier to vary the step size

            // adaptive P&O with variable step size dependedt on power
            if ((delta_P < 0.01f) && (delta_P > -0.01f)) {
                // no significant change in power so hold
                duty = duty;
            } else {
                if (delta_P > 0.0f) {
                    // Power increased
                    if (delta_V > 0.0f) {
                        direction = -1; // Vin increased and P increased so decrease duty as past MPP
                    } else {
                        direction = 1;  // Vin decreased and P increased so increse duty as we are climbing
                    }
                    step_multiplier = 2.0f; // power is rising so step by one up 
                } else {
                    // Power decreased 
                    if (delta_V > 0.0f) { 
                        direction = 1; //Vin increased and P decreased so increase duty as we are climbing
                    } else {
                        direction = -1; //Vin decresed and P decrease so decrease duty as past MPP 
                    }
                    step_multiplier = 3.0f;  // power decreased overall therfore larger step to get back to MPP
                }

                // apply the changes to duty cycle
                duty += direction * step * step_multiplier;
            }

            // edge values to stop railing
            if (duty > 95.0f) duty = 95.0f;
            if (duty < 5.0f)  duty = 5.0f;

            output_pwm(duty);

            // terminal outputs all information (mostly for debugging)
            sprintf(term_msg, 
                "Vin: %.2f V | Iin: %.3f A | P: %.3f W | dP: %.4f | dV: %.4f | Dir: %d | Mult: %.1f | Duty: %.1f%%\n\r",
                avg_Vin, avg_Iin, avg_P,
                delta_P, delta_V,
                direction, step_multiplier, duty);
            print_terminal(term_msg);

            // update the previous and restart
            prev_P = avg_P;
            prev_Vin = avg_Vin;
        }

        led_off();
    }
}

