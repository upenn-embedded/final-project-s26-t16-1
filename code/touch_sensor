#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include "uart.h"

// Initialize the ADC to read from pin PC0
void initialize() {
    // Clear power reduction bit for ADC
    PRR0 &= ~(1 << PRADC);
    
    // Select AVCC as reference voltage and set channel 0
    ADMUX = (1 << REFS0);
    
    // Disable digital input on PC0
    DIDR0 |= (1 << ADC0D);
    
    // Set the ADC clock to pre-scale to 128
    ADCSRA |= (1 << ADEN) | (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2);   
}

// Read a value from the ADC
uint16_t adc_read(uint8_t ch) {
    // Select channel 0 (PC0)
    ADMUX = (ADMUX & 0xF8) | (ch & 0x07);

    // Start single conversion by setting ADSC
    ADCSRA |= (1 << ADSC);

    // Wait for conversion to complete
    while (ADCSRA & (1 << ADSC));

    // Return the 10-bit ADC value 
    return (ADC);
}

int main() {
    initialize();   
    uart_init(); 
    
    uint16_t adc_result;

    while (1) {
        // Read from channel 0 (PC0)
        adc_result = adc_read(0);
        
        // Print the decimal value 
        printf("ADC Value on PC0: %u\n", adc_result);
        
        _delay_ms(250); 
    }
    return 0;
}