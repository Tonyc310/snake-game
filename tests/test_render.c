#include "game.h"
#include "lcd.h"
#include "render.h"
#include "unity.h"

#include <string.h>

/* A fake panel: whatever the renderer draws lands in this frame buffer. */
static uint16_t screen[LCD_HEIGHT][LCD_WIDTH];
static uint16_t window_x;
static uint16_t window_y;
static uint16_t window_width;
static uint32_t window_pixels;
static uint32_t written;

void lcd_begin_pixels(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    TEST_ASSERT_TRUE((x + width) <= LCD_WIDTH);
    TEST_ASSERT_TRUE((y + height) <= LCD_HEIGHT);
    window_x = x;
    window_y = y;
    window_width = width;
    window_pixels = (uint32_t)width * height;
    written = 0u;
}

void lcd_write_pixel(uint16_t colour)
{
    TEST_ASSERT_LESS_THAN_UINT32(window_pixels, written);
    screen[window_y + (written / window_width)][window_x + (written % window_width)] = colour;
    written++;
}

void lcd_end_pixels(void)
{
    TEST_ASSERT_EQUAL_UINT32(window_pixels, written);
}

void lcd_fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t colour)
{
    lcd_begin_pixels(x, y, width, height);
    for (uint32_t i = 0u; i < window_pixels; i++) {
        lcd_write_pixel(colour);
    }
    lcd_end_pixels();
}

void setUp(void)
{
}

void tearDown(void)
{
}

/* The opening snake lies under the banner, so this also covers cells that changed meanwhile. */
static void test_a_closed_banner_leaves_no_trace(void)
{
    static const render_banner_t banner = {"PAUSED", "P TO RESUME"};
    static uint16_t before[LCD_HEIGHT][LCD_WIDTH];
    game_t game;

    game_init(&game, 1u);
    render_init();
    render_game(&game, 0u, NULL);
    memcpy(before, screen, sizeof screen);

    render_game(&game, 0u, &banner);
    TEST_ASSERT_TRUE(memcmp(before, screen, sizeof screen) != 0);
    render_game(&game, 0u, NULL);
    TEST_ASSERT_EQUAL_MEMORY(before, screen, sizeof screen);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_a_closed_banner_leaves_no_trace);
    return UNITY_END();
}
