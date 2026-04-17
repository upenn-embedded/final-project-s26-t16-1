//#define F_CPU 16000000UL
//
//#include <avr/io.h>
//#include <stdio.h>
//#include <stdlib.h> // For atoi()
//#include <util/delay.h>
//#include "uart.h"
//
//void PWM_Init_PD5() {
//    DDRD |= (1 << DDD5);
//    TCCR0A = (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);
//    TCCR0B = (1 << CS01); 
//}
//
//void Set_Motor_Speed_PD5(uint16_t adc_value) {
//    uint8_t duty = (uint8_t)(adc_value >> 2);
//    OCR0B = duty;
//}
//
//int main(void) {
//    uart_init();      
//    PWM_Init_PD5();   
//
//    printf("Motor Controller (PD5) Online.\n");
//
//    char rx_buffer[16];
//    uint8_t index = 0;
//
//    while (1) {
//        // Use the library's receive function directly to read 1 character
//        char c = uart_receive(NULL);
//
//        // Build a string until we hit \n or \r
//        if (c == '\n' || c == '\r') {
//            if (index > 0) { // If we have data, process it
//                rx_buffer[index] = '\0'; // Null-terminate
//                int receivedValue = atoi(rx_buffer); // Convert to int
//                
//                if (receivedValue < 0) receivedValue = 0;
//                Set_Motor_Speed_PD5((uint16_t)receivedValue);
//
//                printf("Received: %d -> Duty: %d/255\n", receivedValue, (receivedValue >> 2));
//                
//                index = 0; // Reset for the next number
//            }
//        } 
//        else if (c >= '0' && c <= '9') {
//            // Only save numbers to the buffer
//            if (index < sizeof(rx_buffer) - 1) {
//                rx_buffer[index++] = c;
//            }
//        }
//    }
//}

//#define F_CPU 16000000UL
//
//#include <avr/io.h>
//#include <stdio.h>
//#include "uart.h"
//
//int main(void) {
//    // Initialize UART at 9600 baud
//    uart_init();      
//
//    // Send a boot message (The ESP32 should catch this and print it to your PC)
//    printf("ATmega328PB Online and Waiting!\n");
//
//    while (1) {
//        // Wait for a single character from the ESP32
//        // uart_receive blocks until a character arrives
//        char receivedChar = uart_receive(NULL);
//
//        // Immediately echo back what we received!
//        printf("Success! ATmega received: %c\n", receivedChar);
//    }
//}

#define F_CPU 16000000UL

#include <avr/io.h>

#define BAUD 9600
#define UBRR_VALUE ((F_CPU / (16UL * BAUD)) - 1)

static void uart0_init(void) {
    UBRR0H = (unsigned char)(UBRR_VALUE >> 8);
    UBRR0L = (unsigned char)(UBRR_VALUE & 0xFF);

    UCSR0A = 0x00;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

static void uart0_putchar(char c) {
    while (!(UCSR0A & (1 << UDRE0))) {
    }
    UDR0 = c;
}

static char uart0_getchar(void) {
    while (!(UCSR0A & (1 << RXC0))) {
    }
    return UDR0;
}

static void uart0_print(const char *s) {
    while (*s) {
        if (*s == '\n') {
            uart0_putchar('\r');
        }
        uart0_putchar(*s);
        s++;
    }
}

int main(void) {
    // PB5 as output for debug LED
    DDRB |= (1 << PB5);
    PORTB &= ~(1 << PB5);

    uart0_init();
    uart0_print("ATmega328PB booted.\n");

    while (1) {
        char c = uart0_getchar();

        if (c == 'X') {
            PORTB ^= (1 << PB5);   // toggle LED
        }

        uart0_print("Got: ");
        uart0_putchar(c);
        uart0_print("\n");
    }
}