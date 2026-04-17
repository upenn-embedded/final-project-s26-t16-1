#define F_CPU 16000000UL

#include <avr/io.h>
#include <stdio.h>
#include <stdint.h>
#include "uart.h"

// Initialize Timer0 for 8-bit Fast PWM on PD5
static void PWM_Init_PD5(void) {
    DDRD |= (1 << DDD5);
    TCCR0A = (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);
    TCCR0B = (1 << CS01);
}

static void Set_Motor_Speed_PD5(uint16_t adc_value) {
    uint8_t duty = (uint8_t)(adc_value >> 2);
    OCR0B = duty;
}

int main(void) {
    int receivedValue = 0;

    uart_init();
    PWM_Init_PD5();

    printf("Motor Controller (PD5) Online.\n");

    while (1) {
        uart_scanf("%d", &receivedValue);

        if (receivedValue < 0) {
            receivedValue = 0;
        }

        Set_Motor_Speed_PD5((uint16_t)receivedValue);
        printf("Received: %d -> Duty: %d/255\n", receivedValue, (receivedValue >> 2));
    }
}
