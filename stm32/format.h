#ifndef FORMAT_H
#define FORMAT_H

#include <stddef.h>
#include <stdint.h>

#define FORMAT_UINT_SIZE 11u /* the ten digits of UINT32_MAX and a terminator */

/** Writes `value` in decimal, NUL-terminated, and returns the number of digits. */
size_t format_uint(char text[FORMAT_UINT_SIZE], uint32_t value);

#endif
