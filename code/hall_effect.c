/*
 * main.c
 * Display spool rotation count on Adafruit 1.8" TFT LCD
 * ATmega328PB Xplained Mini
 *
 * Hall sensor connected to ADC0 (PC0)
 * LCD wired exactly as your diagram
 */

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
    ADMUX = (1 << REFS0);   // AVcc reference, ADC0
    ADCSRA = (1 << ADEN) |
             (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // /128
}

uint16_t ADC_read(uint8_t ch) {
    ADMUX = (ADMUX & 0xF0) | (ch & 0x0F);

    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));

    return ADC;
}

// ---------- MAIN ----------
int main(void) {

    // init LCD
    lcd_init();
    LCD_rotate(1);                 // landscape
    LCD_setScreen(BLACK);
    LCD_brightness(255);

    // init ADC
    ADC_init();
    
    //uart
    uart_init();

    // variables
    uint32_t spool_count = 0;
    uint8_t magnet_seen = 0;
    char buffer[32];

    // title screen
    LCD_drawString(10, 10, "SPOOL COUNTER", YELLOW, BLACK);
    LCD_drawString(10, 35, "Rotations:", WHITE, BLACK);

    while (1) {
        uint16_t val = ADC_read(0);   // read hall sensor

        /*
         No magnet ? 1.0V
         With 5V ADC reference:
         1.0V => 205 counts

         Trigger when magnet moves near sensor.
         Adjust thresholds if needed.
        */

        if (val > 260 && magnet_seen == 0) {
            spool_count++;
            magnet_seen = 1;
        }

        if (val < 230) {
            magnet_seen = 0;
        }

        // erase old number area
        LCD_drawBlock(10, 55, 150, 90, BLACK);

        // print count
        printf(buffer, "%d", spool_count);
        LCD_drawString(10, 60, buffer, GREEN, BLACK);

        // optional raw ADC display
        LCD_drawBlock(10, 100, 150, 120, BLACK);
        printf(buffer, "ADC:%u", val);
        LCD_drawString(10, 105, buffer, CYAN, BLACK);

        _delay_ms(40);
    }
}