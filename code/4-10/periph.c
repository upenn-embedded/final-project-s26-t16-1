#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "uart.h"

void adc_init(void) {
   ADMUX = (1 << REFS0);
   ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t adc_read(void) {
   // Start ADC conversion
   ADCSRA |= (1 << ADSC);
   while (ADCSRA & (1 << ADSC));
   return ADC;
}

int main(void) {
   // Initialize UART
   uart_init();
   
   // Initialize the ADC
   adc_init();

   while (1) {
       // Get the ADC value from PC0
       uint16_t sensorValue = adc_read();
       printf("%u\n", sensorValue);
       _delay_ms(100);
   }

   return 0;
}