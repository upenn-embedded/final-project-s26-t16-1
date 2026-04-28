///* * Project: ATmega328PB to ESP32 ADC Bridge
// * File: main.c
// */
//
//#define F_CPU 16000000UL
//#include <avr/io.h>
//#include <util/delay.h>
//#include <stdio.h>
//#include "uart.h"
//
///**
// * Initialize ADC0 (PC0)
// * Uses AVCC as reference, 128 prescaler for 16MHz clock
// */
//void adc_init(void) {
//    // REFS0: Use AVCC (5V or 3.3V depending on your board)
//    // MUX3..0: 0000 for ADC0 (PC0)
//    ADMUX = (1 << REFS0);
//    
//    // ADEN: Enable ADC
//    // ADPS2..0: Prescaler 128 (16MHz / 128 = 125KHz)
//    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
//}
//
///**
// * Read 10-bit value from ADC0
// */
//uint16_t adc_read(void) {
//    // Start conversion
//    ADCSRA |= (1 << ADSC);
//    
//    // Wait for conversion to complete
//    while (ADCSRA & (1 << ADSC));
//    
//    // Return the result (ADC register contains ADCL + ADCH)
//    return ADC;
//}
//
//int main(void) {
//    // Initialize your UART library
//    // This calls __init_stdout so printf works
//    uart_init();
//    
//    // Initialize the ADC
//    adc_init();
//
//    while (1) {
//        // Get the value from PC0
//        uint16_t sensorValue = adc_read();
//
//        /**
//         * Your ESP32 code uses extractFirstUInt16() which looks for
//         * digits and triggers on '\n'. 
//         * Using printf here maps to your uart_send.
//         */
//        printf("%u\n", sensorValue);
//
//        // 100ms delay - matches the responsiveness of your ESP32 loop
//        _delay_ms(100);
//    }
//
//    return 0;
//}