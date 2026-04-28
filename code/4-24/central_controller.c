#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "uart.h"
#include "lib/ST7735.h"
#include "lib/LCD_GFX.h"

// ---------------- PWM (Motor Control - Timer 2) ----------------
static void pwm_init(void) {
    DDRD |= (1 << DDD3);  
    TCCR2A = (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);
    TCCR2B = (1 << CS21);
    OCR2B = 0;
}

// ---------------- PEDAL SENSITIVITY ----------------
#define PEDAL_MIN   400
#define PEDAL_MAX   950

// ---------------- SOFTWARE STITCH ESTIMATION ----------------
#define MAX_STITCHES_PER_SECOND  8UL

volatile uint32_t stitch_count_x100 = 0;
volatile uint8_t current_duty = 0;

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

// ---------------- UART RX INTERRUPT BUFFER ----------------

#define UART_RX_BUF_SIZE 64

volatile char uart_rx_buf[UART_RX_BUF_SIZE];
volatile uint8_t uart_rx_head = 0;
volatile uint8_t uart_rx_tail = 0;

ISR(USART_RX_vect) {
    char c = UDR0;

    uint8_t next_head = (uart_rx_head + 1) % UART_RX_BUF_SIZE;

    if (next_head != uart_rx_tail) {
        uart_rx_buf[uart_rx_head] = c;
        uart_rx_head = next_head;
    }
}

static uint8_t uart_available_buffered(void) {
    return uart_rx_head != uart_rx_tail;
}

static char uart_read_buffered(void) {
    char c = 0;

    if (uart_rx_head != uart_rx_tail) {
        c = uart_rx_buf[uart_rx_tail];
        uart_rx_tail = (uart_rx_tail + 1) % UART_RX_BUF_SIZE;
    }

    return c;
}

static void uart_rx_interrupt_enable(void) {
    UCSR0B |= (1 << RXCIE0);
}

// Initialize timer 1 to CTC mode
static void timer1_init_1ms(void) {
    TCCR1A = 0;
    TCCR1B = (1 << WGM12);
    OCR1A = 249;
    TIMSK1 = (1 << OCIE1A);
    TCCR1B |= (1 << CS11) | (1 << CS10);
}

ISR(TIMER1_COMPA_vect) {
    static uint32_t accum = 0;

    if (current_duty > 0) {
        accum += ((uint32_t)current_duty *
                  MAX_STITCHES_PER_SECOND *
                  100UL);

        while (accum >= (255UL * 1000UL)) {
            stitch_count_x100++;
            accum -= (255UL * 1000UL);
        }
    } else {
        accum = 0;
    }
}

// ---------------- MOTOR DUTY UPDATE ----------------

static uint8_t map_pedal_to_duty(uint16_t val) {
    if (val <= PEDAL_MIN) {
        return 0;
    } else if (val >= PEDAL_MAX) {
        return 255;
    } else {
        return (uint8_t)(((uint32_t)(val - PEDAL_MIN) * 255UL) /
                         (PEDAL_MAX - PEDAL_MIN));
    }
}

static void process_pedal_uart(void) {
    static char rx_buffer[16];
    static uint8_t index = 0;

    static uint16_t last_good_val = 0;
    static uint8_t have_last_good = 0;

    while (uart_available_buffered()) {
        char c = uart_read_buffered();

        if (isdigit((unsigned char)c)) {
            if (index < sizeof(rx_buffer) - 1) {
                rx_buffer[index++] = c;
            } else {
                index = 0;
            }
        }
        else if (c == '\n' || c == '\r') {
            if (index > 0) {
                rx_buffer[index] = '\0';

                long val = atol(rx_buffer);

                if (val < 0) {
                    val = 0;
                }

                if (val > 1023) {
                    val = 1023;
                }
                if (have_last_good &&
                    last_good_val > 450 &&
                    val < 80) {
                } else {
                    last_good_val = (uint16_t)val;
                    have_last_good = 1;

                    uint8_t duty = map_pedal_to_duty((uint16_t)val);

                    OCR2B = duty;
                    current_duty = duty;
                }

                index = 0;
            }
        }
        else {
            index = 0;
        }
    }
}

static uint32_t get_stitch_count_x100_safe(void) {
    uint32_t copy;

    cli();
    copy = stitch_count_x100;
    sei();

    return copy;
}

// ---------------- MAIN ----------------
int main(void) {

    // Initialize LCD
    _delay_ms(200);
    lcd_init();
    LCD_rotate(1);
    LCD_setScreen(BLACK);
    LCD_brightness(255);

    // Initialize other peripherals
    uart_init();
    pwm_init();
    timer1_init_1ms();

    uart_rx_interrupt_enable();
    sei();

    printf("System Ready\n");

    // LCD static UI
    LCD_drawString(10, 8,   "SEWING MACHINE", YELLOW,  BLACK);
    LCD_drawString(10, 25,  "Stitches:",      WHITE,   BLACK);
    LCD_drawString(10, 42,  "Sewn:",          WHITE,   BLACK);
    LCD_drawString(10, 59,  "Left:",          WHITE,   BLACK);
    LCD_drawString(10, 76,  "Thread:",        WHITE,   BLACK);
    LCD_drawString(10, 115, "Duty:",          MAGENTA, BLACK);

    draw_bar_outline();

    uint16_t lcd_div = 0;

    uint32_t last_stitch_display = 0xFFFFFFFFUL;
    uint8_t last_percent_display = 255;
    uint8_t last_duty_display = 255;

    while (1) {

        // Process incoming pedal UART values
        process_pedal_uart();

        // LCD update
        lcd_div++;

        if (lcd_div >= 8000) {
            lcd_div = 0;

            char buffer[32];

            uint32_t local_stitch_count_x100 = get_stitch_count_x100_safe();

            uint32_t stitch_count_whole = local_stitch_count_x100 / 100UL;

            uint32_t thread_used_x10 =
                stitch_count_whole * THREAD_PER_STITCH_X10;

            uint32_t thread_remaining_x10 =
                (thread_used_x10 >= TOTAL_THREAD_MM_X10)
                ? 0UL
                : TOTAL_THREAD_MM_X10 - thread_used_x10;

            uint32_t fabric_sewn_cm_x10 =
                (stitch_count_whole * STITCH_LENGTH_X10) / 10UL;

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

            if (stitch_count_whole != last_stitch_display) {
                sprintf(buffer, "%lu      ", stitch_count_whole);
                LCD_drawString(70, 25, buffer, GREEN, BLACK);

                sprintf(buffer, "%lu.%lu cm   ",
                        fabric_sewn_cm_x10 / 10,
                        fabric_sewn_cm_x10 % 10);
                LCD_drawString(46, 42, buffer, WHITE, BLACK);

                sprintf(buffer, "%lu.%lu cm   ",
                        fabric_left_cm_x10 / 10,
                        fabric_left_cm_x10 % 10);
                LCD_drawString(46, 59, buffer, WHITE, BLACK);

                last_stitch_display = stitch_count_whole;
            }

            if (percent != last_percent_display) {
                sprintf(buffer, "%lu cm (%u%%)   ",
                        thread_remaining_cm, percent);
                LCD_drawString(58, 76, buffer, bar_color, BLACK);

                draw_bar_fill(percent, bar_color);

                last_percent_display = percent;
            }

            if (current_duty != last_duty_display) {
                sprintf(buffer, "%u   ", current_duty);
                LCD_drawString(50, 115, buffer, MAGENTA, BLACK);

                last_duty_display = current_duty;
            }
        }
    }
}