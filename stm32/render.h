#ifndef RENDER_H
#define RENDER_H

#include "game.h"

#define RENDER_CELL 16u /* pixels per board cell */
#define RENDER_HUD 48u  /* height of the score bar above the board */

/** Clears the screen and forgets what was drawn. Call after lcd_init(). */
void render_init(void);

/** Draws the game, sending only the cells that changed since the last call. */
void render_game(const game_t *game);

#endif
