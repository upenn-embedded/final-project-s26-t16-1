#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include "uart.h"

void ADC_init(void) {
    // AVcc as reference, select ADC0 (PC0)
    ADMUX = (1 << REFS0);

    // Enable ADC, prescaler = 128
    // ADC clock = 16MHz / 128 = 125kHz (good)
    ADCSRA = (1 << ADEN) |
             (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t ADC_read(uint8_t channel) {
    // clear old channel bits, keep voltage ref
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);

    // start conversion
    ADCSRA |= (1 << ADSC);

    // wait until done
    while (ADCSRA & (1 << ADSC));

    return ADC;
}

void init(void) {
    // PC0 input
    DDRC &= ~(1 << DDC0);

    // NO pull-up for analog input
    PORTC &= ~(1 << PORTC0);

    ADC_init();
}

int main(void) {
    uint16_t sensor_val;

    init();
    UART_init(103);

    while (1) {
        sensor_val = ADC_read(0);   // read ADC0 / PC0

        // adjust threshold as needed
        if (sensor_val > 600) {
            UART_putstring((char *)"magnet detected\n");
        } 

        _delay_ms(50);
    }
}