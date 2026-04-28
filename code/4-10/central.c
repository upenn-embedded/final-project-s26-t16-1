#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <ctype.h>

#define BAUD 9600
#define UBRR_VALUE ((F_CPU / (16UL * BAUD)) - 1)

// Initialize UART0
static void uart0_init(void) {
   UBRR0H = (unsigned char)(UBRR_VALUE >> 8);
   UBRR0L = (unsigned char)(UBRR_VALUE & 0xFF);
   UCSR0B = (1 << RXEN0) | (1 << TXEN0);
   UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8N1 stop bit
}

// Initialize timer 0 for fast PWM (PD5)
static void pwm_init(void) {
   // Set PD5 as output
   DDRD |= (1 << DDD5);

   // Set timer pre-scaler and mode
   TCCR0A = (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);
   TCCR0B = (1 << CS01);

   // Set initial duty cycle to 0%
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
   DDRB |= (1 << PB5);
   uart0_print("PWM Controller Booted. Ready for ESP32 data...\n");

   char rx_buffer[16];
   uint8_t index = 0;
   
   while (1) {
       if (UCSR0A & (1 << RXC0)) {
           char c = UDR0;

           if (isdigit(c)) {
               if (index < sizeof(rx_buffer) - 1) {
                   rx_buffer[index++] = c;
               }
           } 
           else if (index > 0) { 
               rx_buffer[index] = '\0';

               // Convert string to long
               long adc_val = atol(rx_buffer);

               // Constrain value to 10-bit max (1023) j
               if (adc_val > 1023) adc_val = 1023;

               // Scale 10-bit to 8-bit to set PWM
               OCR0B = (uint8_t)(adc_val >> 2);

               // Toggle feedback LED
               PORTB ^= (1 << PB5);

               index = 0; 
           }
       }
   }

}