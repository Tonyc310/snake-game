#include "render.h"

#include "font.h"
#include "format.h"
#include "lcd.h"

#include <limits.h>
#include <stdbool.h>
#include <string.h>

/* The desktop game's colours. */
#define BACKGROUND LCD_RGB(17u, 24u, 39u)
#define BOARD LCD_RGB(31u, 41u, 55u)
#define BODY LCD_RGB(22u, 163u, 74u)
#define HEAD LCD_RGB(74u, 222u, 128u)
#define FOOD LCD_RGB(239u, 68u, 68u)
#define TEXT LCD_RGB(243u, 244u, 246u)
#define MUTED LCD_RGB(156u, 163u, 175u)

/* Gaps around the squares, so neighbouring segments read as separate cells and food looks small. */
#define SEGMENT_INSET 1u
#define FOOD_INSET 4u

#define LARGE 2u /* text scale on the score line */
#define SMALL 1u
#define HUD_MARGIN 8u
#define SCORE_LINE_Y 8u
#define HINT_LINE_Y 32u

/* The score line reads "SCORE 123  BEST 123"; these are its character columns. */
#define SCORE_LABEL_COLUMN 0u
#define SCORE_COLUMN 6u
#define BEST_LABEL_COLUMN 11u
#define BEST_COLUMN 16u
#define NUMBER_DIGITS 3u

/* The banner covers whole cells, so whatever it hid comes back by redrawing those cells. */
#define BANNER_LEFT 2
#define BANNER_TOP 4
#define BANNER_COLUMNS 16
#define BANNER_ROWS 4
#define BANNER_X ((uint16_t)(BANNER_LEFT * RENDER_CELL))
#define BANNER_Y ((uint16_t)(RENDER_HUD + (BANNER_TOP * RENDER_CELL)))
#define BANNER_WIDTH ((uint16_t)(BANNER_COLUMNS * RENDER_CELL))
#define BANNER_HEIGHT ((uint16_t)(BANNER_ROWS * RENDER_CELL))
#define BANNER_TITLE_Y ((uint16_t)(BANNER_Y + 16u))
#define BANNER_HINT_Y ((uint16_t)(BANNER_Y + 40u))

_Static_assert((GAME_WIDTH * RENDER_CELL) == LCD_WIDTH, "the board spans the screen");
_Static_assert(RENDER_HUD + (GAME_HEIGHT * RENDER_CELL) == LCD_HEIGHT,
               "the board fills the screen below the score bar");
_Static_assert(GAME_MAX_LENGTH - 3 < 1000, "a score fits in three digits");
_Static_assert(((BANNER_LEFT + BANNER_COLUMNS) <= GAME_WIDTH) &&
                   ((BANNER_TOP + BANNER_ROWS) <= GAME_HEIGHT),
               "the banner lies on the board");

typedef enum { CELL_UNDRAWN, CELL_EMPTY, CELL_FOOD, CELL_BODY, CELL_HEAD } cell_t;

/* What the panel shows now. A step changes only about three cells, so only those get sent. */
static cell_t shown[GAME_HEIGHT][GAME_WIDTH];
static unsigned shown_score;
static unsigned shown_best;
static const render_banner_t *shown_banner;

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

/* Glyphs are drawn opaque, so new text covers old text without clearing it first. */
static void draw_text(uint16_t x, uint16_t y, uint16_t scale, uint16_t colour, const char *text)
{
    const uint16_t size = (uint16_t)(FONT_SIZE * scale);

    for (; *text != '\0'; text++) {
        const uint8_t *glyph = font_glyph(*text);

        lcd_begin_pixels(x, y, size, size);
        for (uint16_t row = 0u; row < size; row++) {
            for (uint16_t column = 0u; column < size; column++) {
                const bool lit = ((glyph[row / scale] >> (column / scale)) & 1u) != 0u;

                lcd_write_pixel(lit ? colour : BACKGROUND);
            }
        }
        lcd_end_pixels();
        x = (uint16_t)(x + size);
    }
}

static uint16_t score_line_x(unsigned column)
{
    return (uint16_t)(HUD_MARGIN + (column * FONT_SIZE * LARGE));
}

/* Padded with spaces, so a shorter number fully covers a longer one, as after a restart. */
static void draw_number(unsigned column, unsigned value)
{
    char text[FORMAT_UINT_SIZE];
    size_t length = format_uint(text, value);

    while (length < NUMBER_DIGITS) {
        text[length] = ' ';
        length++;
    }
    text[length] = '\0';
    draw_text(score_line_x(column), SCORE_LINE_Y, LARGE, TEXT, text);
}

static void draw_centred(uint16_t y, uint16_t scale, uint16_t colour, const char *text)
{
    const uint16_t width = (uint16_t)(strlen(text) * FONT_SIZE * scale);

    draw_text((uint16_t)((LCD_WIDTH - width) / 2u), y, scale, colour, text);
}

static void draw_banner(const render_banner_t *banner)
{
    lcd_fill_rect(BANNER_X, BANNER_Y, BANNER_WIDTH, BANNER_HEIGHT, BACKGROUND);
    draw_centred(BANNER_TITLE_Y, LARGE, TEXT, banner->title);
    draw_centred(BANNER_HINT_Y, SMALL, MUTED, banner->hint);
}

static bool under_banner(int x, int y)
{
    return (x >= BANNER_LEFT) && (x < (BANNER_LEFT + BANNER_COLUMNS)) && (y >= BANNER_TOP) &&
           (y < (BANNER_TOP + BANNER_ROWS));
}

void render_init(void)
{
    lcd_fill_rect(0u, 0u, LCD_WIDTH, LCD_HEIGHT, BACKGROUND);
    draw_text(score_line_x(SCORE_LABEL_COLUMN), SCORE_LINE_Y, LARGE, TEXT, "SCORE");
    draw_text(score_line_x(BEST_LABEL_COLUMN), SCORE_LINE_Y, LARGE, TEXT, "BEST");
    draw_text(HUD_MARGIN, HINT_LINE_Y, SMALL, MUTED, "ARROWS/WASD STEER  P PAUSE  R RESTART");

    (void)memset(shown, 0, sizeof shown); /* CELL_UNDRAWN, so the next frame draws every cell */
    shown_score = UINT_MAX;               /* nor are any numbers drawn yet */
    shown_best = UINT_MAX;
    shown_banner = NULL;
}

void render_game(const game_t *game, unsigned best, const render_banner_t *banner)
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
            if ((banner != NULL) && under_banner(x, y)) {
                shown[y][x] = CELL_UNDRAWN; /* hidden, so it's redrawn once the banner goes */
            } else if (wanted[y][x] != shown[y][x]) {
                draw_cell(x, y, wanted[y][x]);
                shown[y][x] = wanted[y][x];
            }
        }
    }
    if (banner != shown_banner) {
        if (banner != NULL) {
            draw_banner(banner);
        }
        shown_banner = banner;
    }

    if (game->score != shown_score) {
        draw_number(SCORE_COLUMN, game->score);
        shown_score = game->score;
    }
    if (best != shown_best) {
        draw_number(BEST_COLUMN, best);
        shown_best = best;
    }
}
