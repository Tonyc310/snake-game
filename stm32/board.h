#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>
#include <stdint.h>

/** Enables clocks, routes the console UART, SPI1 and the LCD's pins, and starts the 1 ms tick. */
void board_init(void);

/** Milliseconds since board_init(); wraps after about 49 days. */
uint32_t board_uptime_ms(void);

/** Waits at least `ms` milliseconds, sleeping between ticks. */
void board_delay_ms(uint32_t ms);

/** Drives the LCD's chip select (PB6, active low). */
void board_lcd_select(bool selected);

/** Drives the LCD's data/command line (PC7): true sends data, false a command. */
void board_lcd_data_mode(bool data);

/** Holds the LCD in reset (PA9, active low) while `asserted`. */
void board_lcd_reset(bool asserted);

#endif
