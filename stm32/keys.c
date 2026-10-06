#include "keys.h"

#define ESCAPE 0x1Bu

enum { STATE_IDLE, STATE_ESCAPE, STATE_SEQUENCE };

/* Arrow keys arrive as ESC [ A..D, or ESC O A..D from a terminal in application mode. */
static keys_input_t arrow(uint8_t final)
{
    switch (final) {
    case 'A':
        return KEYS_UP;
    case 'B':
        return KEYS_DOWN;
    case 'C':
        return KEYS_RIGHT;
    case 'D':
        return KEYS_LEFT;
    default:
        return KEYS_NONE;
    }
}

/* Either case, so Caps Lock doesn't stop the game. */
static keys_input_t letter(uint8_t byte)
{
    switch (byte) {
    case 'w':
    case 'W':
        return KEYS_UP;
    case 's':
    case 'S':
        return KEYS_DOWN;
    case 'a':
    case 'A':
        return KEYS_LEFT;
    case 'd':
    case 'D':
        return KEYS_RIGHT;
    case 'p':
    case 'P':
        return KEYS_PAUSE;
    case 'r':
    case 'R':
        return KEYS_RESTART;
    default:
        return KEYS_NONE;
    }
}

keys_input_t keys_decode(keys_decoder_t *decoder, uint8_t byte)
{
    if (decoder->state == STATE_SEQUENCE) {
        /* Parameter bytes, as in ESC [ 1 ; 5 A for Ctrl+Up, come before the final byte. */
        if ((byte >= 0x20u) && (byte <= 0x3Fu)) {
            return KEYS_NONE;
        }
        decoder->state = STATE_IDLE;
        return arrow(byte);
    }
    if (decoder->state == STATE_ESCAPE) {
        if ((byte == '[') || (byte == 'O')) {
            decoder->state = STATE_SEQUENCE;
            return KEYS_NONE;
        }
        /* A lone Esc press: the byte after it is an ordinary key. */
        decoder->state = STATE_IDLE;
    }
    if (byte == ESCAPE) {
        decoder->state = STATE_ESCAPE;
        return KEYS_NONE;
    }
    return letter(byte);
}
