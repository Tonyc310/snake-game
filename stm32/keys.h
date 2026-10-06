#ifndef KEYS_H
#define KEYS_H

#include <stdint.h>

typedef enum {
    KEYS_NONE,
    KEYS_UP,
    KEYS_DOWN,
    KEYS_LEFT,
    KEYS_RIGHT,
    KEYS_PAUSE,
    KEYS_RESTART,
} keys_input_t;

typedef struct {
    uint8_t state; /* how far into an escape sequence the last bytes went */
} keys_decoder_t;

/** Decodes one byte from a serial terminal; returns the key it completes, or KEYS_NONE. */
keys_input_t keys_decode(keys_decoder_t *decoder, uint8_t byte);

#endif
