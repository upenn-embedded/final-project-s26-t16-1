#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "uart.h"
#include "lib/ST7735.h"
#include "lib/LCD_GFX.h"

// ---------------- PWM (Motor Control - Timer2) ----------------
static void pwm_init(void) {
    DDRD |= (1 << DDD3);  // PD3 = OC2B

    TCCR2A = (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);
    TCCR2B = (1 << CS21); // prescaler 8

    OCR2B = 0;
}

// ---------------- ADC (Fixed channel PC5) ----------------
void ADC_init(void) {
    ADMUX = (1 << REFS0) | (1 << MUX2) | (1 << MUX0); // PC5
    DIDR0 |= (1 << ADC5D);

    ADCSRA = (1 << ADEN) |
             (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t ADC_read(void) {
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;
}

// ---------------- HALL SENSOR THRESHOLDS ----------------
#define HALL_HIGH   260
#define HALL_LOW    180

// ---------------- THREAD / DISPLAY CONFIG ----------------
#define STITCH_LENGTH_X10       25UL
#define FABRIC_THICKNESS_X10    1UL
#define SPOOL_YARDAGE           250UL

#define THREAD_PER_STITCH_X10   (2UL * FABRIC_THICKNESS_X10 + STITCH_LENGTH_X10)

#define TOTAL_THREAD_MM         ((SPOOL_YARDAGE * 9144UL) / 10UL)
#define TOTAL_THREAD_MM_X10     (TOTAL_THREAD_MM * 10UL)

// ---------------- PROGRESS BAR ----------------
#define BAR_X 10
#define BAR_Y 95
#define BAR_W 140
#define BAR_H 11

static void draw_bar_outline(void) {
    LCD_drawBlock(BAR_X, BAR_Y, BAR_X + BAR_W + 1, BAR_Y + 1, WHITE);
    LCD_drawBlock(BAR_X, BAR_Y + BAR_H, BAR_X + BAR_W + 1, BAR_Y + BAR_H + 1, WHITE);
    LCD_drawBlock(BAR_X, BAR_Y, BAR_X + 1, BAR_Y + BAR_H + 1, WHITE);
    LCD_drawBlock(BAR_X + BAR_W, BAR_Y, BAR_X + BAR_W + 1, BAR_Y + BAR_H + 1, WHITE);
}

static void draw_bar_fill(uint8_t percent, uint16_t color) {
    uint16_t inner_w = BAR_W - 1;
    uint16_t fill_px = ((uint16_t)percent * inner_w) / 100U;

    if (fill_px > 0) {
        LCD_drawBlock(BAR_X + 1, BAR_Y + 1,
                      BAR_X + 1 + fill_px, BAR_Y + BAR_H,
                      color);
    }
    if (fill_px < inner_w) {
        LCD_drawBlock(BAR_X + 1 + fill_px, BAR_Y + 1,
                      BAR_X + 1 + inner_w, BAR_Y + BAR_H,
                      BLACK);
    }
}

// ---------------- MAIN ----------------
int main(void) {

    // --- LCD FIRST ---
    _delay_ms(200);
    lcd_init();
    LCD_rotate(1);
    LCD_setScreen(BLACK);
    LCD_brightness(255);

    // --- Other peripherals ---
    uart_init();
    pwm_init();
    ADC_init();

    printf("System Ready\n");

    // UART buffer
    char rx_buffer[16];
    uint8_t index = 0;

    // Hall tracking
    uint32_t stitch_count = 0;
    uint8_t magnet_seen = 0;

    // LCD static UI
    LCD_drawString(10, 8,   "SEWING MACHINE", YELLOW,  BLACK);
    LCD_drawString(10, 25,  "Stitches:",      WHITE,   BLACK);
    LCD_drawString(10, 42,  "Sewn:",          WHITE,   BLACK);
    LCD_drawString(10, 59,  "Left:",          WHITE,   BLACK);
    LCD_drawString(10, 76,  "Thread:",        WHITE,   BLACK);
    LCD_drawString(10, 115, "ADC:",           MAGENTA, BLACK);

    draw_bar_outline();

    uint16_t lcd_div = 0;

    while (1) {

        // -------- FAST: UART + PWM --------
        if (UCSR0A & (1 << RXC0)) {
            char c = UDR0;

            if (isdigit(c)) {
                if (index < sizeof(rx_buffer) - 1) {
                    rx_buffer[index++] = c;
                }
            }
            else if (index > 0) {
                rx_buffer[index] = '\0';

                long val = atol(rx_buffer);
                if (val > 1023) val = 1023;

                OCR2B = (uint8_t)(val >> 2);

                printf("PWM: %ld\n", val);

                index = 0;
            }
        }

        // -------- ADC --------
        uint16_t adc_val = ADC_read();

        // -------- HALL EDGE DETECT --------
        if (adc_val > HALL_HIGH && magnet_seen == 0) {
            stitch_count++;
            magnet_seen = 1;
        }
        if (adc_val < HALL_LOW) {
            magnet_seen = 0;
        }

        // -------- LCD UPDATE (THROTTLED) --------
        lcd_div++;
        if (lcd_div >= 3000) {
            lcd_div = 0;

            char buffer[32];

            uint32_t thread_used_x10 = stitch_count * THREAD_PER_STITCH_X10;

            uint32_t thread_remaining_x10 =
                (thread_used_x10 >= TOTAL_THREAD_MM_X10)
                ? 0UL
                : TOTAL_THREAD_MM_X10 - thread_used_x10;

            uint32_t fabric_sewn_cm_x10 =
                (stitch_count * STITCH_LENGTH_X10) / 10UL;

            uint32_t stitches_left =
                thread_remaining_x10 / THREAD_PER_STITCH_X10;

            uint32_t fabric_left_cm_x10 =
                (stitches_left * STITCH_LENGTH_X10) / 10UL;

            uint32_t thread_remaining_cm =
                thread_remaining_x10 / 100UL;

            uint8_t percent =
                (uint8_t)((thread_remaining_x10 * 100UL) / TOTAL_THREAD_MM_X10);

            uint16_t bar_color =
                (percent > 30) ? GREEN :
                (percent > 10) ? YELLOW : RED;

            sprintf(buffer, "%lu      ", stitch_count);
            LCD_drawString(70, 25, buffer, GREEN, BLACK);

            sprintf(buffer, "%lu.%lu cm   ",
                    fabric_sewn_cm_x10 / 10,
                    fabric_sewn_cm_x10 % 10);
            LCD_drawString(46, 42, buffer, WHITE, BLACK);

            sprintf(buffer, "%lu.%lu cm   ",
                    fabric_left_cm_x10 / 10,
                    fabric_left_cm_x10 % 10);
            LCD_drawString(46, 59, buffer, WHITE, BLACK);

            sprintf(buffer, "%lu cm (%u%%)   ",
                    thread_remaining_cm, percent);
            LCD_drawString(58, 76, buffer, bar_color, BLACK);

            draw_bar_fill(percent, bar_color);

            sprintf(buffer, "%u   ", adc_val);
            LCD_drawString(34, 115, buffer, MAGENTA, BLACK);
        }
    }
}