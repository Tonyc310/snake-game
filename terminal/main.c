#include "game.h"
#include "render.h"

#include <curses.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#define STEP_MS 150

static long now_ms(void)
{
    struct timespec now;

    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec * 1000L + now.tv_nsec / 1000000L;
}

/* Returns false once the player quits. */
static bool handle_key(game_t *game, int key)
{
    switch (key) {
    case KEY_UP:
    case 'w':
        game_turn(game, GAME_UP);
        break;
    case KEY_DOWN:
    case 's':
        game_turn(game, GAME_DOWN);
        break;
    case KEY_LEFT:
    case 'a':
        game_turn(game, GAME_LEFT);
        break;
    case KEY_RIGHT:
    case 'd':
        game_turn(game, GAME_RIGHT);
        break;
    case 'q':
        return false;
    default:
        break;
    }
    return true;
}

static const char *status_message(const game_t *game)
{
    switch (game->status) {
    case GAME_OVER:
        return "game over - q to quit";
    case GAME_WON:
        return "you filled the board! - q to quit";
    default:
        return "arrows or WASD to steer, q to quit";
    }
}

int main(void)
{
    game_t game;
    bool running = true;

    game_init(&game, (uint32_t)time(NULL));

    initscr();
    cbreak(); /* keys arrive as they're pressed, not after Enter */
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    render_draw(&game, status_message(&game));

    long next_step = now_ms() + STEP_MS;
    while (running) {
        /* Wait for a key, but never past the next step, so the snake keeps a steady pace. */
        long wait = next_step - now_ms();
        timeout(wait > 0 ? (int)wait : 0);

        int key = getch();
        if (key != ERR) {
            running = handle_key(&game, key);
        }
        if (now_ms() >= next_step) {
            game_step(&game);
            render_draw(&game, status_message(&game));
            next_step += STEP_MS;
        }
    }

    endwin();
    return 0;
}
