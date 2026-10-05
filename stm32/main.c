#include "board.h"
#include "game.h"
#include "lcd.h"
#include "render.h"
#include "stm32f4xx.h"

/* The F446 has no hardware random-number generator, so the opening board is the same every boot. */
#define OPENING_SEED 1u

static game_t game; /* about 2 KB, kept off the stack */

int main(void)
{
    board_init();
    lcd_init();
    render_init();
    game_init(&game, OPENING_SEED);
    render_game(&game);

    for (;;) {
        __WFI(); /* nothing else runs yet, so sleep */
    }
}
