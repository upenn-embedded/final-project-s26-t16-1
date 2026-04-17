#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

#define BAUD 9600
#define UBRR_VALUE ((F_CPU / (16UL * BAUD)) - 1)

// Initialize UART0
static void uart0_init(void) {
    UBRR0H = (unsigned char)(UBRR_VALUE >> 8);
    UBRR0L = (unsigned char)(UBRR_VALUE & 0xFF);
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8N1
}

// Initialize Timer 0 for Fast PWM on PD5 (OC0B)
static void pwm_init(void) {
    // Set PD5 as output
    DDRD |= (1 << DDD5);

    /* TCCR0A:
       - COM0B1: Clear OC0B on Compare Match, set at BOTTOM (Non-inverting mode)
       - WGM01 & WGM00: Fast PWM Mode (Mode 3)
    */
    TCCR0A = (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);

    /* TCCR0B:
       - CS01: Prescaler 8 (16MHz / 8 / 256 = ~7.8kHz PWM frequency)
    */
    TCCR0B = (1 << CS01);

    // Initial duty cycle 0%
    OCR0B = 0;
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
    
    // Debug LED on PB5
//    DDRB |= (1 << PB5);
    uart0_print("PWM Controller Booted. Ready for ESP32 data...\n");

    char rx_buffer[16];
    uint8_t index = 0;

    while (1) {
        // Check if data is available (Non-blocking)
        if (UCSR0A & (1 << RXC0)) {
            char c = UDR0;

            if (isdigit(c)) {
                if (index < sizeof(rx_buffer) - 1) {
                    rx_buffer[index++] = c;
                }
            } 
            else if (c == '\n' || c == '\r') {
                if (index > 0) {
                    rx_buffer[index] = '\0';
                    long adc_val = atol(rx_buffer);
                    
                    // Scale 10-bit (0-1023) to 8-bit (0-255)
                    uint8_t duty = (uint8_t)(adc_val / 4);
                    
                    // Update PWM Duty Cycle
                    OCR0B = duty;
                    
                    // Optional: Toggle LED to show valid packet received
//                    PORTB ^= (1 << PB5);
                    
                    index = 0; // Reset buffer
                }
            }
        }
    }
}
