#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#define BAUD 9600
#define UBRR_VALUE ((F_CPU / (16UL * BAUD)) - 1)

static void uart0_init(void) {
    UBRR0H = (unsigned char)(UBRR_VALUE >> 8);
    UBRR0L = (unsigned char)(UBRR_VALUE & 0xFF);
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

static void adc_init(void) {
    ADMUX = (1 << REFS0) | (1 << MUX2) | (1 << MUX0); // Monitoring PC5
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

static uint16_t adc_read(void) {
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;
}

static void pwm_init(void) {
    // Set PD3 (OC2B) as output
    DDRD |= (1 << DDD3);

    /* TCCR2A:
       - COM2B1: Clear OC2B on Compare Match, set at BOTTOM (Non-inverting)
       - WGM21 & WGM20: Fast PWM Mode (Mode 3)
    */
    TCCR2A = (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);

    /* TCCR2B:
       - CS21: Prescaler 8 (matches your original ~7.8kHz frequency)
    */
    TCCR2B = (1 << CS21); 

    // Initial duty cycle 0
    OCR2B = 0;
}

static void uart0_putchar(char c) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = c;
}

static void uart0_print(const char *s) {
    while (*s) {
        if (*s == '\n') uart0_putchar('\r');
        uart0_putchar(*s++);
    }
}

int main(void) {
    uart0_init();
    pwm_init();
    adc_init();
    
    // Debug LED on PB5
    DDRB |= (1 << PB5); 
    uart0_print("System Online. Timer 2 PWM (PD3) and PC5 Monitoring Active.\n");

    char rx_buffer[16];
    char tx_buffer[32];
    uint8_t index = 0;
    uint32_t adc_timer = 0; 
    
    while (1) {
        // --- 1. NON-BLOCKING ADC PRINT ---
        if (adc_timer++ >= 50000) { 
            uint16_t val = adc_read();
            sprintf(tx_buffer, "PC5: %u\n", val);
            uart0_print(tx_buffer);
            adc_timer = 0;
        }

        // --- 2. CONSTANT UART CHECK ---
        if (UCSR0A & (1 << RXC0)) {
            char c = UDR0;

            if (isdigit(c)) {
                if (index < sizeof(rx_buffer) - 1) {
                    rx_buffer[index++] = c;
                }
            } 
            else if (index > 0) { 
                rx_buffer[index] = '\0';
                long pwm_input = atol(rx_buffer);
                
                if (pwm_input > 1023) pwm_input = 1023;
                
                // Set PWM Duty Cycle for Timer 2 (8-bit)
                OCR2B = (uint8_t)(pwm_input >> 2); 
                
                PORTB ^= (1 << PB5); // Toggle feedback LED
                index = 0; 
            }
        }
    }
}
