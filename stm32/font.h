#ifndef FONT_H
#define FONT_H

#include <stdint.h>

#define FONT_SIZE 8u /* glyphs are 8x8 pixels */

/** The 8 rows of `c`'s glyph, leftmost pixel in bit 0; characters outside ' '..'_' are blank. */
const uint8_t *font_glyph(char c);

#endif
