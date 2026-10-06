#ifndef RENDER_H
#define RENDER_H

#include "game.h"

#define RENDER_CELL 16u /* pixels per board cell */
#define RENDER_HUD 48u  /* height of the score bar above the board */

/** Clears the screen and draws the score bar's labels; the next frame redraws everything. */
void render_init(void);

/** Draws the game and the session's best score, sending only what changed since the last call. */
void render_game(const game_t *game, unsigned best);

#endif
