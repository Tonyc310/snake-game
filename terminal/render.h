#ifndef RENDER_H
#define RENDER_H

#include "game.h"

/** Draws the board, snake, food, score, and session best, with `message` underneath. */
void render_draw(const game_t *game, unsigned best, const char *message);

#endif
