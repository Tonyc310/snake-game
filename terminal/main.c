#include "game.h"
#include "render.h"

#include <curses.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

typedef struct {
    game_t game;
    unsigned best; /* best score this session */
    bool paused;
    bool running;
} session_t;

static long now_ms(void)
{
    struct timespec now;

    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec * 1000L + now.tv_nsec / 1000000L;
}

static uint32_t new_seed(void)
{
    /* Milliseconds differ between quick restarts; time() would replay a game within a second. */
    return (uint32_t)now_ms();
}

static void steer(session_t *session, game_direction_t direction)
{
    if (!session->paused) {
        game_turn(&session->game, direction);
    }
}

static void handle_key(session_t *session, int key)
{
    switch (key) {
    case KEY_UP:
    case 'w':
        steer(session, GAME_UP);
        break;
    case KEY_DOWN:
    case 's':
        steer(session, GAME_DOWN);
        break;
    case KEY_LEFT:
    case 'a':
        steer(session, GAME_LEFT);
        break;
    case KEY_RIGHT:
    case 'd':
        steer(session, GAME_RIGHT);
        break;
    case 'p':
        /* Pausing only means something mid-game. */
        session->paused = !session->paused && session->game.status == GAME_PLAYING;
        break;
    case 'r':
        game_init(&session->game, new_seed());
        session->paused = false;
        break;
    case 'q':
        session->running = false;
        break;
    default:
        break;
    }
}

/* Messages must fit within the board's width, which sets the minimum terminal size. */
static const char *status_message(const session_t *session)
{
    if (session->paused) {
        return "paused - p resume, q quit";
    }
    switch (session->game.status) {
    case GAME_OVER:
        return "game over - r restart, q quit";
    case GAME_WON:
        return "board filled, you win! r restart, q quit";
    default:
        return "arrows/WASD steer, p pause, q quit";
    }
}

int main(void)
{
    session_t session = {.best = 0, .paused = false, .running = true};

    game_init(&session.game, new_seed());

    initscr();
    cbreak(); /* keys arrive as they're pressed, not after Enter */
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    long next_step = now_ms() + game_step_ms(&session.game);
    while (session.running) {
        if (!render_draw(&session.game, session.best, status_message(&session)) &&
            session.game.status == GAME_PLAYING) {
            /* Don't let the snake run while the board can't be seen. */
            session.paused = true;
        }

        /* Wait for a key, but never past the next step, so the snake keeps a steady pace. */
        long wait = next_step - now_ms();
        timeout(wait > 0 ? (int)wait : 0);

        int key = getch();
        if (key != ERR) {
            handle_key(&session, key);
        }

        if (session.paused || session.game.status != GAME_PLAYING) {
            /* Nothing moves, so keep the next step a full interval away for when play resumes. */
            next_step = now_ms() + game_step_ms(&session.game);
        } else if (now_ms() >= next_step) {
            game_step(&session.game);
            if (session.game.score > session.best) {
                session.best = session.game.score;
            }
            next_step += game_step_ms(&session.game);
        }
    }

    endwin();
    return 0;
}
