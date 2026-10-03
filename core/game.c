#include "game.h"

#define START_LENGTH 3u

static const game_point_t deltas[] = {
    [GAME_UP] = {0, -1},
    [GAME_DOWN] = {0, 1},
    [GAME_LEFT] = {-1, 0},
    [GAME_RIGHT] = {1, 0},
};

/* Lets game_turn() refuse a turn straight back into the neck. */
static const game_direction_t opposites[] = {
    [GAME_UP] = GAME_DOWN,
    [GAME_DOWN] = GAME_UP,
    [GAME_LEFT] = GAME_RIGHT,
    [GAME_RIGHT] = GAME_LEFT,
};

void game_init(game_t *game)
{
    /* body[0] is the tail and body[START_LENGTH - 1] the head, so the snake faces right. */
    for (size_t i = 0; i < START_LENGTH; i++) {
        int behind_head = (int)(START_LENGTH - 1u - i);

        game->body[i] = (game_point_t){GAME_WIDTH / 2 - behind_head, GAME_HEIGHT / 2};
    }
    game->head = START_LENGTH - 1u;
    game->length = START_LENGTH;
    game->moving = GAME_RIGHT;
    game->heading = GAME_RIGHT;
}

void game_turn(game_t *game, game_direction_t direction)
{
    /* Checked against the last step, not the last key, so two quick keys can't reverse it. */
    if (direction != opposites[game->moving]) {
        game->heading = direction;
    }
}

void game_step(game_t *game)
{
    game_point_t head = game->body[game->head];
    game_point_t delta = deltas[game->heading];

    /* The tail needs no work: whatever lies past `length` from the head drops off the snake. */
    game->head = (game->head + 1u) % GAME_MAX_LENGTH;
    game->body[game->head] = (game_point_t){head.x + delta.x, head.y + delta.y};
    game->moving = game->heading;
}

game_point_t game_segment(const game_t *game, size_t index)
{
    /* Adding GAME_MAX_LENGTH first keeps the unsigned subtraction from going below zero. */
    return game->body[(game->head + GAME_MAX_LENGTH - index) % GAME_MAX_LENGTH];
}
