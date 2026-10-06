#ifndef RENDER_H
#define RENDER_H

#include "game.h"

#define RENDER_CELL 16u /* pixels per board cell */
#define RENDER_HUD 48u  /* height of the score bar above the board */

typedef struct {
    const char *title; /* at most 16 characters */
    const char *hint;  /* at most 32 */
} render_banner_t;

/** Clears the screen and draws the score bar's labels; the next frame redraws everything. */
void render_init(void);

/** Draws the game, the best score and `banner` (unless NULL), sending only what changed. */
void render_game(const game_t *game, unsigned best, const render_banner_t *banner);

#endif
