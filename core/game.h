#ifndef GAME_H
#define GAME_H

#include <stddef.h>

#define GAME_WIDTH 20
#define GAME_HEIGHT 12
#define GAME_MAX_LENGTH (GAME_WIDTH * GAME_HEIGHT) /* a snake that fills the board */

typedef enum { GAME_UP, GAME_DOWN, GAME_LEFT, GAME_RIGHT } game_direction_t;

typedef struct {
    int x;
    int y; /* grows downward, like screen rows */
} game_point_t;

typedef struct {
    game_point_t body[GAME_MAX_LENGTH]; /* circular; body[head] is the head */
    size_t head;
    size_t length;
    game_direction_t moving;  /* direction of the last step */
    game_direction_t heading; /* direction of the next step */
} game_t;

/** Starts a 3-segment snake in the middle of the board, heading right. */
void game_init(game_t *game);

/** Sets the direction of the next step; a turn back into the neck is ignored. */
void game_turn(game_t *game, game_direction_t direction);

/** Moves the snake one cell. */
void game_step(game_t *game);

/** Segment `index` of the snake, counted from the head (0); `index` must be below the length. */
game_point_t game_segment(const game_t *game, size_t index);

#endif
