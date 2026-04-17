#define F_CPU 16000000UL

#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include "uart.h"

// Initialize Timer0 for 8-bit Fast PWM on PD5
void PWM_Init_PD5() {
    // Set PD5 (OC0B) as an output pin
    DDRD |= (1 << DDD5);

    // Mode 3: Fast PWM (Top at 0xFF / 255)
    // WGM01=1, WGM00=1
    // COM0B1: Non-inverting PWM on OC0B (PD5)
    TCCR0A = (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);
    
    // CS01: Prescaler of 8
    TCCR0B = (1 << CS01); 
}

void Set_Motor_Speed_PD5(uint16_t adc_value) {
    // ADC is 10-bit (0-1023), Timer0 is 8-bit (0-255).
    // Divide by 4 to scale the value correctly.
    uint8_t duty = (uint8_t)(adc_value >> 2);
    
    // OCR0B controls the duty cycle on PD5
    OCR0B = duty;
}

int main(void) {
    uart_init();      // Redirects stdin/stdout
    PWM_Init_PD5();   // Setup Timer0 on PD5

    int receivedValue = 0;
`
    printf("Motor Controller (PD5) Online.\n");

    while (1) {
        // uart_scanf uses the logic in your uart.c to grab the number
        uart_scanf("%d", &receivedValue);

        if (receivedValue < 0) receivedValue = 0;
        
        // Scale and update PWM
        Set_Motor_Speed_PD5((uint16_t)receivedValue);

        // Debug back to PC via ESP32
        printf("Received: %d -> Duty: %d/255\n", receivedValue, (receivedValue >> 2));
    }
}