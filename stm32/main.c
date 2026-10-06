#include "board.h"
#include "game.h"
#include "keys.h"
#include "lcd.h"
#include "render.h"
#include "stm32f4xx.h"
#include "uart.h"

#include <stdbool.h>
#include <string.h>

#define CONSOLE_BAUD 115200u

/* The F446 has no hardware random-number generator, so the opening board is the same every boot. */
#define OPENING_SEED 1u

static game_t game; /* about 2 KB, kept off the stack */
static keys_decoder_t keys;

static void print(const char *text)
{
    /* A full buffer cuts the line short rather than stall the game. */
    (void)uart_write(UART_CONSOLE, (const uint8_t *)text, strlen(text));
}

/* printf isn't linked into this firmware, so the score is formatted by hand. */
static void print_score(const char *label, unsigned score)
{
    char digits[10]; /* enough for any 32-bit value */
    size_t count = 0u;

    do {
        digits[count] = (char)('0' + (score % 10u));
        count++;
        score /= 10u;
    } while (score > 0u);

    print(label);
    while (count > 0u) {
        count--;
        (void)uart_write(UART_CONSOLE, (const uint8_t *)&digits[count], 1u);
    }
    print("\r\n");
}

static game_direction_t direction_for(keys_input_t key)
{
    switch (key) {
    case KEYS_UP:
        return GAME_UP;
    case KEYS_DOWN:
        return GAME_DOWN;
    case KEYS_LEFT:
        return GAME_LEFT;
    default: /* KEYS_RIGHT; KEYS_NONE never gets this far */
        return GAME_RIGHT;
    }
}

static void report(unsigned score_before)
{
    if (game.status == GAME_OVER) {
        print_score("game over, score ", game.score);
    } else if (game.status == GAME_WON) {
        print_score("board filled, you win! score ", game.score);
    } else if (game.score > score_before) {
        print_score("score ", game.score);
    }
}

int main(void)
{
    bool started = false;
    uint32_t last_step_ms = 0u;

    board_init();
    uart_init(UART_CONSOLE, CONSOLE_BAUD);
    lcd_init();
    render_init();
    game_init(&game, OPENING_SEED);
    render_game(&game);
    print("snake: steer with the arrow keys or WASD\r\n");

    for (;;) {
        uint8_t byte;

        while (uart_read(UART_CONSOLE, &byte, 1u) == 1u) {
            const keys_input_t key = keys_decode(&keys, byte);

            if (key == KEYS_NONE) {
                continue;
            }
            if (!started) {
                /* The player's timing is the only randomness on hand, so it seeds the game. */
                game_init(&game, board_uptime_ms());
                last_step_ms = board_uptime_ms();
                started = true;
            }
            game_turn(&game, direction_for(key));
        }

        /* Unsigned subtraction stays correct when the millisecond counter wraps. */
        if (started && (game.status == GAME_PLAYING) &&
            ((board_uptime_ms() - last_step_ms) >= game_step_ms(&game))) {
            const unsigned score_before = game.score;

            last_step_ms += game_step_ms(&game);
            game_step(&game);
            render_game(&game);
            report(score_before);
        }
        __WFI(); /* until the next tick or a received byte */
    }
}
