#include "board.h"
#include "format.h"
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

static game_t game;   /* about 2 KB, kept off the stack */
static unsigned best; /* best score this session */
static bool started;  /* the opening board waits for the first key */
static bool paused;
static uint32_t last_step_ms;
static keys_decoder_t keys;

static void print(const char *text)
{
    /* A full buffer cuts the line short rather than stall the game. */
    (void)uart_write(UART_CONSOLE, (const uint8_t *)text, strlen(text));
}

static void print_score(const char *label, unsigned score)
{
    char digits[FORMAT_UINT_SIZE];

    (void)format_uint(digits, score);
    print(label);
    print(digits);
    print("\r\n");
}

static const render_banner_t *banner(void)
{
    static const render_banner_t waiting = {"SNAKE", "PRESS AN ARROW KEY OR WASD"};
    static const render_banner_t pause = {"PAUSED", "P TO RESUME"};
    static const render_banner_t over = {"GAME OVER", "R TO RESTART"};
    static const render_banner_t won = {"YOU WIN!", "R TO RESTART"};

    if (!started) {
        return &waiting;
    }
    if (paused) {
        return &pause;
    }
    switch (game.status) {
    case GAME_OVER:
        return &over;
    case GAME_WON:
        return &won;
    default:
        return NULL;
    }
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
    default: /* KEYS_RIGHT; the other keys never get this far */
        return GAME_RIGHT;
    }
}

static void restart(void)
{
    /* The player's timing is the only randomness on hand, so it seeds each game. */
    game_init(&game, board_uptime_ms());
    started = true;
    paused = false;
}

static void handle(keys_input_t key)
{
    switch (key) {
    case KEYS_PAUSE:
        /* Pausing only means something mid-game. */
        paused = !paused && started && (game.status == GAME_PLAYING);
        break;
    case KEYS_RESTART:
        restart();
        break;
    default:
        if (!started) {
            restart();
        }
        if (!paused) {
            game_turn(&game, direction_for(key));
        }
        break;
    }
}

static void step(void)
{
    const unsigned score_before = game.score;

    game_step(&game);
    if (game.score > best) {
        best = game.score;
    }
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
    board_init();
    uart_init(UART_CONSOLE, CONSOLE_BAUD);
    lcd_init();
    render_init();
    game_init(&game, OPENING_SEED);
    render_game(&game, best, banner());
    print("snake: arrow keys or WASD to steer, P to pause, R to restart\r\n");

    for (;;) {
        bool changed = false;
        uint8_t byte;

        while (uart_read(UART_CONSOLE, &byte, 1u) == 1u) {
            const keys_input_t key = keys_decode(&keys, byte);

            if (key != KEYS_NONE) {
                handle(key);
                changed = true;
            }
        }

        if (!started || paused || (game.status != GAME_PLAYING)) {
            /* Nothing moves, so the next step stays a full interval away for when play resumes. */
            last_step_ms = board_uptime_ms();
        } else if ((board_uptime_ms() - last_step_ms) >= game_step_ms(&game)) {
            /* The unsigned subtraction above stays correct when the millisecond counter wraps. */
            last_step_ms += game_step_ms(&game);
            step();
            changed = true;
        }

        if (changed) {
            render_game(&game, best, banner());
        }
        __WFI(); /* until the next tick or a received byte */
    }
}
