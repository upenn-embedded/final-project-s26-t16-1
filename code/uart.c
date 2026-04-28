#include <stdio.h>
#include "uart.h"
#include <avr/io.h>
#include <stdarg.h>
#include <string.h>

static unsigned char uart_read_byte(void)
{
    unsigned char status;
    unsigned char data;

    while (1) {
        while (!(UCSR0A & (1 << RXC0)));

        status = UCSR0A;
        data = UDR0;

        if (!(status & ((1 << FE0) | (1 << DOR0) | (1 << UPE0)))) {
            return data;
        }
    }
}

void uart_init()
{
    /*Set baud rate */
    UBRR0H = (unsigned char)(UART_BAUD_PRESCALER>>8);
    UBRR0L = (unsigned char)UART_BAUD_PRESCALER;
    //Enable receiver and transmitter
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);
    /* Match the ESP32 bridge configuration: 8 data bits, no parity, 1 stop bit */
    UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);
    
    __init_stdout(uart_send);
    __init_stdin(uart_receive);
}

int uart_send(char data, FILE* stream)
{
    // Wait for empty transmit buffer
    while(!(UCSR0A & (1<<UDRE0)));
    // Put data into buffer and send data
    UDR0 = data;
    return 0;
}

int uart_receive(FILE* stream)
{
    (void)stream;
    return uart_read_byte();
}

void determine_line_ending() {
    char c;
    printf("Press Enter to detect the line ending style...\n");

    while(1)
    {
        c = uart_receive(NULL);
        if (c == '\r') {
            printf("\\r (CR) detected.\n");
        } else if (c == '\n') {
            printf("\\n (LF) detected.\n");
        } else {
            printf("Unknown line ending.\n");
        }
    }
}

// Only integer (%d), char (%c), and string (%s) format specifiers have been implemented
#if !defined(CR) && !defined(LF) && !defined(CRLF)
#error "No line termination defined! #define one out of CR, LF, or CRLF"
#else
#ifdef MAX_STRING_LENGTH
void uart_scanf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    const char* p = format;
    char buffer[MAX_STRING_LENGTH];
    int i = 0, num = 0;
    char c;

    while (*p) {
        if (*p == '%') 
        {
            p++;
            switch (*p) 
            {
                case 'd':
                {
                    num = 0;
                    while(1) {
                        c = uart_receive(NULL);
                        if(c >= '0' && c <= '9') 
                        {
                            num = num * 10 + (c - '0');
                        }
                        #if defined(CR) || defined(CRLF)
                        else if(c == '\r')
                        {
                            #ifdef CRLF
                            uart_receive(NULL);
                            #endif
                            break;
                        }
                        #endif
                        #ifdef LF
                        else if(c == '\n')
                        {
                            break;
                        }
                        #endif
                    }
                    int *int_ptr = va_arg(args, int*);
                    *int_ptr = num;
                    break;
                }
                case 's':
                {
                    i = 0;
                    while(1) 
                    {
                        c = uart_receive(NULL);

                        #if defined(CR) || defined(CRLF) 
                        if(c == '\r')
                        {
                            #ifdef CRLF
                            uart_receive(NULL);
                            #endif
                            buffer[i] = '\0';
                            break;
                        }
                        #endif
                        #ifdef LF
                        if(c == '\n')
                        {
                            buffer[i] = '\0';
                            break;
                        }
                        #endif

                        if(i < MAX_STRING_LENGTH - 1 && c != '\r' && c != '\n') 
                        {
                            buffer[i++] = c;
                        }
                    }
                    buffer[i] = '\0';
                    char *str_ptr = va_arg(args, char*);
                    strcpy(str_ptr, buffer);
                    break;
                }
                case 'c':
                {
                    char *char_ptr = va_arg(args, char*);
                    while(1)
                    {
                        c = uart_receive(NULL);
                        #if defined(CR) || defined(CRLF)
                        if(c == '\r') 
                        {
                            #ifdef CRLF
                            c = uart_receive(NULL);
                            #endif
                            break;
                        }
                        #endif
                        #ifdef LF
                        if(c == '\n') 
                        {
                            break;
                        }
                        #endif
                        *char_ptr = c;
                        break;
                    }
                    break;
                }
            }
        }
        p++;
    }
    va_end(args);
}
#else
#error "MAX_STRING_LENGTH undefined"
#endif
#endif
