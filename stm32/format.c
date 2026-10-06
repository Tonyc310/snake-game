#include "format.h"

/* printf isn't linked into this firmware, so numbers are formatted by hand. */
size_t format_uint(char text[FORMAT_UINT_SIZE], uint32_t value)
{
    char reversed[FORMAT_UINT_SIZE - 1u];
    size_t count = 0u;

    do {
        reversed[count] = (char)('0' + (value % 10u));
        count++;
        value /= 10u;
    } while (value > 0u);

    for (size_t i = 0u; i < count; i++) {
        text[i] = reversed[count - 1u - i];
    }
    text[count] = '\0';
    return count;
}
