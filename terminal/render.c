#include "render.h"

#include <curses.h>

/* Terminal characters are about twice as tall as wide, so each game cell takes two columns. */
#define CELL_WIDTH 2
#define BOARD_COLUMNS (GAME_WIDTH * CELL_WIDTH)

static void draw_border(void)
{
    mvaddch(0, 0, ACS_ULCORNER);
    mvhline(0, 1, ACS_HLINE, BOARD_COLUMNS);
    mvaddch(0, BOARD_COLUMNS + 1, ACS_URCORNER);
    mvvline(1, 0, ACS_VLINE, GAME_HEIGHT);
    mvvline(1, BOARD_COLUMNS + 1, ACS_VLINE, GAME_HEIGHT);
    mvaddch(GAME_HEIGHT + 1, 0, ACS_LLCORNER);
    mvhline(GAME_HEIGHT + 1, 1, ACS_HLINE, BOARD_COLUMNS);
    mvaddch(GAME_HEIGHT + 1, BOARD_COLUMNS + 1, ACS_LRCORNER);
}

static void draw_cell(game_point_t cell, chtype glyph)
{
    /* +1 on both axes steps inside the border. */
    mvaddch(cell.y + 1, cell.x * CELL_WIDTH + 1, glyph);
}

void render_draw(const game_t *game, unsigned best, const char *message)
{
    erase();
    draw_border();
    if (game->has_food) {
        draw_cell(game->food, '*');
    }
    /* Tail first, so the head is drawn last and always shows. */
    for (size_t i = game->length; i-- > 1u;) {
        draw_cell(game_segment(game, i), 'o');
    }
    draw_cell(game_segment(game, 0), '@');
    mvprintw(GAME_HEIGHT + 2, 0, "score %u   best %u", game->score, best);
    mvaddstr(GAME_HEIGHT + 3, 0, message);
    refresh();
}
