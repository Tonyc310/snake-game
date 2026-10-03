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

static bool same_cell(game_point_t a, game_point_t b)
{
    return a.x == b.x && a.y == b.y;
}

static bool on_snake(const game_t *game, game_point_t cell)
{
    for (size_t i = 0; i < game->length; i++) {
        if (same_cell(game_segment(game, i), cell)) {
            return true;
        }
    }
    return false;
}

/* xorshift32: tiny and deterministic, which is all food placement needs. */
static uint32_t next_random(game_t *game)
{
    uint32_t x = game->rng;

    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    game->rng = x;
    return x;
}

/* Picks the n-th free cell rather than retrying random ones, which could spin on a full board. */
static void place_food(game_t *game)
{
    size_t free_cells = GAME_MAX_LENGTH - game->length;

    game->has_food = free_cells > 0u;
    if (!game->has_food) {
        return;
    }
    size_t skip = next_random(game) % free_cells;
    for (int y = 0; y < GAME_HEIGHT; y++) {
        for (int x = 0; x < GAME_WIDTH; x++) {
            game_point_t cell = {x, y};

            if (!on_snake(game, cell) && skip-- == 0u) {
                game->food = cell;
                return;
            }
        }
    }
}

void game_init(game_t *game, uint32_t seed)
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
    game->score = 0;
    /* Zero would keep xorshift at zero forever. */
    game->rng = seed != 0u ? seed : 1u;
    place_food(game);
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
    game_point_t next = {head.x + delta.x, head.y + delta.y};

    /* The tail needs no work: whatever lies past `length` from the head drops off the snake. */
    game->head = (game->head + 1u) % GAME_MAX_LENGTH;
    game->body[game->head] = next;
    game->moving = game->heading;

    if (game->has_food && same_cell(next, game->food)) {
        /* Growing is just keeping the tail that this step would have dropped. */
        game->length++;
        game->score++;
        place_food(game);
    }
}

game_point_t game_segment(const game_t *game, size_t index)
{
    /* Adding GAME_MAX_LENGTH first keeps the unsigned subtraction from going below zero. */
    return game->body[(game->head + GAME_MAX_LENGTH - index) % GAME_MAX_LENGTH];
}
