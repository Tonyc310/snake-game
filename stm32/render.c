#include "render.h"

#include "lcd.h"

#include <string.h>

/* The desktop game's colours. */
#define BACKGROUND LCD_RGB(17u, 24u, 39u)
#define BOARD LCD_RGB(31u, 41u, 55u)
#define BODY LCD_RGB(22u, 163u, 74u)
#define HEAD LCD_RGB(74u, 222u, 128u)
#define FOOD LCD_RGB(239u, 68u, 68u)

/* Gaps around the squares, so neighbouring segments read as separate cells and food looks small. */
#define SEGMENT_INSET 1u
#define FOOD_INSET 4u

_Static_assert((GAME_WIDTH * RENDER_CELL) == LCD_WIDTH, "the board spans the screen");
_Static_assert(RENDER_HUD + (GAME_HEIGHT * RENDER_CELL) == LCD_HEIGHT,
               "the board fills the screen below the score bar");

typedef enum { CELL_UNDRAWN, CELL_EMPTY, CELL_FOOD, CELL_BODY, CELL_HEAD } cell_t;

/* What the panel shows now. A step changes only about three cells, so only those get sent. */
static cell_t shown[GAME_HEIGHT][GAME_WIDTH];

static void fill_square(uint16_t left, uint16_t top, uint16_t inset, uint16_t colour)
{
    const uint16_t size = (uint16_t)(RENDER_CELL - (2u * inset));

    lcd_fill_rect((uint16_t)(left + inset), (uint16_t)(top + inset), size, size, colour);
}

static void draw_cell(int x, int y, cell_t cell)
{
    const uint16_t left = (uint16_t)((unsigned)x * RENDER_CELL);
    const uint16_t top = (uint16_t)(RENDER_HUD + ((unsigned)y * RENDER_CELL));

    fill_square(left, top, 0u, BOARD);
    switch (cell) {
    case CELL_FOOD:
        fill_square(left, top, FOOD_INSET, FOOD);
        break;
    case CELL_BODY:
        fill_square(left, top, SEGMENT_INSET, BODY);
        break;
    case CELL_HEAD:
        fill_square(left, top, SEGMENT_INSET, HEAD);
        break;
    default:
        break;
    }
}

void render_init(void)
{
    lcd_fill_rect(0u, 0u, LCD_WIDTH, LCD_HEIGHT, BACKGROUND);
    (void)memset(shown, 0, sizeof shown); /* CELL_UNDRAWN, so the next frame draws every cell */
}

void render_game(const game_t *game)
{
    cell_t wanted[GAME_HEIGHT][GAME_WIDTH];

    for (int y = 0; y < GAME_HEIGHT; y++) {
        for (int x = 0; x < GAME_WIDTH; x++) {
            wanted[y][x] = CELL_EMPTY;
        }
    }
    if (game->has_food) {
        wanted[game->food.y][game->food.x] = CELL_FOOD;
    }
    for (size_t i = 1u; i < game->length; i++) {
        const game_point_t segment = game_segment(game, i);

        wanted[segment.y][segment.x] = CELL_BODY;
    }
    const game_point_t head = game_segment(game, 0u);
    wanted[head.y][head.x] = CELL_HEAD;

    for (int y = 0; y < GAME_HEIGHT; y++) {
        for (int x = 0; x < GAME_WIDTH; x++) {
            if (wanted[y][x] != shown[y][x]) {
                draw_cell(x, y, wanted[y][x]);
                shown[y][x] = wanted[y][x];
            }
        }
    }
}
