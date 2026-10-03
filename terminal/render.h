#ifndef RENDER_H
#define RENDER_H

#include "game.h"

#include <stdbool.h>

/** Draws the game with `message` below; if it can't fit, asks for a bigger terminal and returns
 * false. */
bool render_draw(const game_t *game, unsigned best, const char *message);

#endif
