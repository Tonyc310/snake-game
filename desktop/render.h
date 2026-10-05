#ifndef RENDER_H
#define RENDER_H

#include "game.h"

#include <SDL3/SDL.h>

#define RENDER_CELL 32 /* logical pixels per game cell */
#define RENDER_HUD 40  /* height of the score bar above the board */
#define RENDER_WIDTH (GAME_WIDTH * RENDER_CELL)
#define RENDER_HEIGHT (RENDER_HUD + GAME_HEIGHT * RENDER_CELL)

/* A message shown over the dimmed board, such as GAME OVER. */
typedef struct {
    const char *title;
    const char *hint;
} render_banner_t;

/** Draws score bar, board, food, and snake, plus `banner` if it isn't NULL. Logical pixels. */
void render_game(SDL_Renderer *renderer, const game_t *game, unsigned best,
                 const render_banner_t *banner);

#endif
