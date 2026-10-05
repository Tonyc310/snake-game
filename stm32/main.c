#include "board.h"
#include "lcd.h"
#include "stm32f4xx.h"

#define BACKGROUND LCD_RGB(17u, 24u, 39u) /* the desktop game's colours */

int main(void)
{
    board_init();
    lcd_init();
    lcd_fill_rect(0u, 0u, LCD_WIDTH, LCD_HEIGHT, BACKGROUND);

    for (;;) {
        __WFI(); /* nothing else runs yet, so sleep */
    }
}
