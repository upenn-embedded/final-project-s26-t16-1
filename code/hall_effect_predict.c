#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>

#include "uart.h"
#include "lib/ST7735.h"
#include "lib/LCD_GFX.h"

// ---------- SPOOL / STITCH CONFIG ----------
// Lengths stored as integer mm x10 to preserve 0.1 mm precision
// without using floats.
#define STITCH_LENGTH_X10       25UL      // 2.5 mm
#define FABRIC_THICKNESS_X10    1UL       // 0.1 mm
#define SPOOL_YARDAGE           250UL     // yards on spool

// Thread burned per stitch (lockstitch geometry):
// one stitch length on top + two passes through the fabric.
#define THREAD_PER_STITCH_X10   (2UL * FABRIC_THICKNESS_X10 + STITCH_LENGTH_X10)  // 27 => 2.7 mm

// 1 yd = 914.4 mm. 250 yd * 9144 / 10 = 228600 mm exactly.
#define TOTAL_THREAD_MM         ((SPOOL_YARDAGE * 9144UL) / 10UL)   // 228600
#define TOTAL_THREAD_MM_X10     (TOTAL_THREAD_MM * 10UL)            // 2286000

// ---------- EDGE DETECT THRESHOLDS ----------
#define HALL_HIGH   220
#define HALL_LOW    240

// ---------- PROGRESS BAR GEOMETRY ----------
#define BAR_X       10
#define BAR_Y       95
#define BAR_W       140
#define BAR_H       11

// ---------- ADC ----------
void ADC_init(void) {
    ADMUX = (1 << REFS0);
    DIDR0 |= (1 << ADC5D);
    ADCSRA = (1 << ADEN) |
             (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t ADC_read(uint8_t ch) {
    ADMUX = (ADMUX & 0xF0) | (ch & 0x0F);
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;
}

// ---------- BAR HELPERS ----------
static void draw_bar_outline(void) {
    // top, bottom, left, right 1-pixel edges
    LCD_drawBlock(BAR_X,           BAR_Y,             BAR_X + BAR_W + 1, BAR_Y + 1,             WHITE);
    LCD_drawBlock(BAR_X,           BAR_Y + BAR_H,     BAR_X + BAR_W + 1, BAR_Y + BAR_H + 1,     WHITE);
    LCD_drawBlock(BAR_X,           BAR_Y,             BAR_X + 1,         BAR_Y + BAR_H + 1,     WHITE);
    LCD_drawBlock(BAR_X + BAR_W,   BAR_Y,             BAR_X + BAR_W + 1, BAR_Y + BAR_H + 1,     WHITE);
}

static void draw_bar_fill(uint8_t percent, uint16_t color) {
    // inner fill region is (BAR_X+1, BAR_Y+1) .. (BAR_X+BAR_W, BAR_Y+BAR_H)
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

// ---------- MAIN ----------
int main(void) {

    // ---------- INIT ----------
    lcd_init();
    LCD_rotate(1);
    LCD_setScreen(BLACK);
    LCD_brightness(255);

    ADC_init();
    uart_init();

    // ---------- VARIABLES ----------
    uint32_t stitch_count = 0;
    uint8_t  magnet_seen  = 0;
    char     buffer[32];

    // ---------- STATIC TEXT ----------
    LCD_drawString(10, 8,   "SEWING MACHINE", YELLOW,  BLACK);
    LCD_drawString(10, 25,  "Stitches:",      WHITE,   BLACK);
    LCD_drawString(10, 42,  "Sewn:",          WHITE,   BLACK);
    LCD_drawString(10, 59,  "Left:",          WHITE,   BLACK);
    LCD_drawString(10, 76,  "Thread:",        WHITE,   BLACK);
    LCD_drawString(10, 115, "ADC:",           MAGENTA, BLACK);

    draw_bar_outline();

    // ---------- LOOP ----------
    while (1) {

        uint16_t val = ADC_read(5);

        // rising edge -> one stitch
        if (val > HALL_HIGH && magnet_seen == 0) {
            stitch_count++;
            magnet_seen = 1;
            printf("Stitch: %lu\n", stitch_count);
        }
        // falling edge re-arms detector
        if (val < HALL_LOW) {
            magnet_seen = 0;
        }

        // ---------- DERIVED VALUES ----------
        uint32_t thread_used_x10      = stitch_count * THREAD_PER_STITCH_X10;
        uint32_t thread_remaining_x10 = (thread_used_x10 >= TOTAL_THREAD_MM_X10)
                                        ? 0UL
                                        : TOTAL_THREAD_MM_X10 - thread_used_x10;

        // fabric sewn so far, in 0.1 cm units: stitches * 2.5 mm = stitches * 0.25 cm
        uint32_t fabric_sewn_cm_x10   = (stitch_count * STITCH_LENGTH_X10) / 10UL;

        // stitches the remaining thread can still make
        uint32_t stitches_left        = thread_remaining_x10 / THREAD_PER_STITCH_X10;
        uint32_t fabric_left_cm_x10   = (stitches_left * STITCH_LENGTH_X10) / 10UL;

        uint32_t thread_remaining_cm  = thread_remaining_x10 / 100UL;

        uint8_t  percent              = (uint8_t)((thread_remaining_x10 * 100UL) / TOTAL_THREAD_MM_X10);

        uint16_t bar_color;
        if      (percent > 30) bar_color = GREEN;
        else if (percent > 10) bar_color = YELLOW;
        else                   bar_color = RED;

        // ---------- DISPLAY ----------
        sprintf(buffer, "%lu      ", stitch_count);
        LCD_drawString(70, 25, buffer, GREEN, BLACK);

        sprintf(buffer, "%lu.%lu cm   ",
                (unsigned long)(fabric_sewn_cm_x10 / 10UL),
                (unsigned long)(fabric_sewn_cm_x10 % 10UL));
        LCD_drawString(46, 42, buffer, WHITE, BLACK);

        sprintf(buffer, "%lu.%lu cm   ",
                (unsigned long)(fabric_left_cm_x10 / 10UL),
                (unsigned long)(fabric_left_cm_x10 % 10UL));
        LCD_drawString(46, 59, buffer, WHITE, BLACK);

        sprintf(buffer, "%lu cm (%u%%)   ",
                (unsigned long)thread_remaining_cm, percent);
        LCD_drawString(58, 76, buffer, bar_color, BLACK);

        draw_bar_fill(percent, bar_color);

        sprintf(buffer, "%u   ", val);
        LCD_drawString(34, 115, buffer, MAGENTA, BLACK);

        _delay_ms(30);
    }
}
