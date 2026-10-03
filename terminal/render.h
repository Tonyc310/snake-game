#ifndef RENDER_H
#define RENDER_H

#include "game.h"

/** Draws the board, snake, food, and score, with `message` underneath. */
void render_draw(const game_t *game, const char *message);

#endif
