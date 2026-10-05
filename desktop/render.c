#include "render.h"

static const SDL_Color BACKGROUND = {17, 24, 39, 255};
static const SDL_Color BOARD = {31, 41, 55, 255};
static const SDL_Color BODY = {22, 163, 74, 255};
static const SDL_Color HEAD = {74, 222, 128, 255};
static const SDL_Color FOOD = {239, 68, 68, 255};

static void set_color(SDL_Renderer *renderer, SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

/* `inset` leaves a gap around the square, so neighbouring segments read as separate cells. */
static void fill_cell(SDL_Renderer *renderer, game_point_t cell, float inset)
{
    SDL_FRect square = {
        (float)(cell.x * RENDER_CELL) + inset,
        (float)(cell.y * RENDER_CELL) + inset,
        RENDER_CELL - 2.0f * inset,
        RENDER_CELL - 2.0f * inset,
    };

    SDL_RenderFillRect(renderer, &square);
}

void render_game(SDL_Renderer *renderer, const game_t *game)
{
    const SDL_FRect board = {0.0f, 0.0f, RENDER_WIDTH, RENDER_HEIGHT};

    /* The clear color also fills the bars SDL adds when the window's shape doesn't match. */
    set_color(renderer, BACKGROUND);
    SDL_RenderClear(renderer);
    set_color(renderer, BOARD);
    SDL_RenderFillRect(renderer, &board);

    if (game->has_food) {
        set_color(renderer, FOOD);
        fill_cell(renderer, game->food, 7.0f);
    }
    set_color(renderer, BODY);
    for (size_t i = 1; i < game->length; i++) {
        fill_cell(renderer, game_segment(game, i), 2.0f);
    }
    set_color(renderer, HEAD);
    fill_cell(renderer, game_segment(game, 0), 1.0f);
}
