#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>

#include "uart.h"
#include "lib/ST7735.h"
#include "lib/LCD_GFX.h"

// ---------- ADC ----------
void ADC_init(void) {
    // AVcc reference
    ADMUX = (1 << REFS0);

    // disable digital input on PC5 (ADC5)
    DIDR0 |= (1 << ADC5D);

    // enable ADC, prescaler 128
    ADCSRA = (1 << ADEN) |
             (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t ADC_read(uint8_t ch) {
    // select channel
    ADMUX = (ADMUX & 0xF0) | (ch & 0x0F);

    // start conversion
    ADCSRA |= (1 << ADSC);

    // wait until done
    while (ADCSRA & (1 << ADSC));

    return ADC;
}

// ---------- MAIN ----------
int main(void) {

    // ---------- INIT ----------
    lcd_init();
    LCD_rotate(1);               // landscape
    LCD_setScreen(BLACK);
    LCD_brightness(255);

    ADC_init();
    uart_init();

    // ---------- VARIABLES ----------
    uint32_t spool_count = 0;
    uint8_t magnet_seen = 0;
    char buffer[32];

    // ---------- STATIC TEXT ----------
    LCD_drawString(10, 10, "SPOOL COUNTER", YELLOW, BLACK);
    LCD_drawString(10, 35, "Rotations:", WHITE, BLACK);
    LCD_drawString(10, 90, "ADC:", WHITE, BLACK);

    // ---------- LOOP ----------
    while (1) {

        uint16_t val = ADC_read(5);

        /*
         no magnet ~ 1.0V (~205)
         magnet raises value

         adjust thresholds if needed
        */

        // rising edge detect
        if (val > 260 && magnet_seen == 0) {
            spool_count++;
            magnet_seen = 1;

            // debug to UART
            printf("Count: %lu\n", spool_count);
        }

        // falling edge reset
        if (val < 230) {
            magnet_seen = 0;
        }

        // ---------- UPDATE DISPLAY ----------

        // rotations
        sprintf(buffer, "%lu   ", spool_count);  // spaces clear old digits
        LCD_drawString(10, 60, buffer, GREEN, BLACK);

        // ADC value
        sprintf(buffer, "%u   ", val);
        LCD_drawString(60, 90, buffer, CYAN, BLACK);

        _delay_ms(80);
    }
}