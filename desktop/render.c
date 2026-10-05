#include "render.h"

#define BOARD_HEIGHT (GAME_HEIGHT * RENDER_CELL)
/* SDL's built-in debug font is 8x8 pixels, too small to read unscaled. */
#define TEXT_SCALE 2.0f
#define GLYPH ((float)SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * TEXT_SCALE)
#define MARGIN 12.0f

static const SDL_Color BACKGROUND = {17, 24, 39, 255};
static const SDL_Color BOARD = {31, 41, 55, 255};
static const SDL_Color BODY = {22, 163, 74, 255};
static const SDL_Color HEAD = {74, 222, 128, 255};
static const SDL_Color FOOD = {239, 68, 68, 255};
static const SDL_Color TEXT = {243, 244, 246, 255};
static const SDL_Color MUTED = {156, 163, 175, 255};
static const SDL_Color SHADE = {17, 24, 39, 200}; /* translucent, over the board */

static void set_color(SDL_Renderer *renderer, SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

static void draw_text(SDL_Renderer *renderer, float x, float y, const char *text)
{
    /* Scaling the renderer, rather than the font, keeps the glyphs crisp. */
    SDL_SetRenderScale(renderer, TEXT_SCALE, TEXT_SCALE);
    SDL_RenderDebugText(renderer, x / TEXT_SCALE, y / TEXT_SCALE, text);
    SDL_SetRenderScale(renderer, 1.0f, 1.0f);
}

static float text_width(const char *text)
{
    return (float)SDL_strlen(text) * GLYPH;
}

/* `inset` leaves a gap around the square, so neighbouring segments read as separate cells. */
static void fill_cell(SDL_Renderer *renderer, game_point_t cell, float inset)
{
    SDL_FRect square = {
        (float)(cell.x * RENDER_CELL) + inset,
        (float)(RENDER_HUD + cell.y * RENDER_CELL) + inset,
        RENDER_CELL - 2.0f * inset,
        RENDER_CELL - 2.0f * inset,
    };

    SDL_RenderFillRect(renderer, &square);
}

static void draw_hud(SDL_Renderer *renderer, const game_t *game, unsigned best)
{
    const char *controls = "P PAUSE  M MUTE";
    const float y = (RENDER_HUD - GLYPH) / 2.0f;
    char score[48];

    SDL_snprintf(score, sizeof score, "SCORE %u   BEST %u", game->score, best);
    set_color(renderer, TEXT);
    draw_text(renderer, MARGIN, y, score);
    set_color(renderer, MUTED);
    draw_text(renderer, RENDER_WIDTH - MARGIN - text_width(controls), y, controls);
}

static void draw_banner(SDL_Renderer *renderer, const render_banner_t *banner)
{
    const SDL_FRect board = {0.0f, RENDER_HUD, RENDER_WIDTH, BOARD_HEIGHT};
    const float middle = RENDER_HUD + BOARD_HEIGHT / 2.0f;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    set_color(renderer, SHADE);
    SDL_RenderFillRect(renderer, &board);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    /* A solid panel keeps the text readable whatever lies under it, like the snake at the start. */
    const float width = SDL_max(text_width(banner->title), text_width(banner->hint)) + 4.0f * GLYPH;
    const SDL_FRect panel = {(RENDER_WIDTH - width) / 2.0f, middle - 2.5f * GLYPH, width,
                             5.0f * GLYPH};
    set_color(renderer, BACKGROUND);
    SDL_RenderFillRect(renderer, &panel);

    set_color(renderer, TEXT);
    draw_text(renderer, (RENDER_WIDTH - text_width(banner->title)) / 2.0f, middle - 1.5f * GLYPH,
              banner->title);
    set_color(renderer, MUTED);
    draw_text(renderer, (RENDER_WIDTH - text_width(banner->hint)) / 2.0f, middle + 0.5f * GLYPH,
              banner->hint);
}

void render_game(SDL_Renderer *renderer, const game_t *game, unsigned best,
                 const render_banner_t *banner)
{
    const SDL_FRect board = {0.0f, RENDER_HUD, RENDER_WIDTH, BOARD_HEIGHT};

    /* The clear color also fills the bars SDL adds when the window's shape doesn't match. */
    set_color(renderer, BACKGROUND);
    SDL_RenderClear(renderer);
    draw_hud(renderer, game, best);
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

    if (banner != NULL) {
        draw_banner(renderer, banner);
    }
}
