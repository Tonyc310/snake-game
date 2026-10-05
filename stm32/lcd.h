#ifndef LCD_H
#define LCD_H

#include <stdint.h>

#define LCD_WIDTH 320u /* landscape */
#define LCD_HEIGHT 240u

/** Packs 8-bit red, green and blue into the panel's 16-bit RGB565. */
#define LCD_RGB(r, g, b) ((uint16_t)((((r) & 0xF8u) << 8) | (((g) & 0xFCu) << 3) | ((b) >> 3)))

/** Resets the panel and sets it up for landscape RGB565 (~240 ms). Call after board_init(). */
void lcd_init(void);

/** Fills a rectangle with one RGB565 colour, clipped to the screen. Blocks until it's sent. */
void lcd_fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t colour);

#endif
