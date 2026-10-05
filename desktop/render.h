#ifndef RENDER_H
#define RENDER_H

#include "game.h"

#include <SDL3/SDL.h>

#define RENDER_CELL 32 /* logical pixels per game cell */
#define RENDER_WIDTH (GAME_WIDTH * RENDER_CELL)
#define RENDER_HEIGHT (GAME_HEIGHT * RENDER_CELL)

/** Draws the board, food, and snake in RENDER_WIDTH x RENDER_HEIGHT logical pixels. */
void render_game(SDL_Renderer *renderer, const game_t *game);

#endif
