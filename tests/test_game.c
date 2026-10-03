#include "game.h"
#include "unity.h"

#include <stdlib.h>

static game_t game;

void setUp(void)
{
    game_init(&game);
}

void tearDown(void)
{
}

static void expect_head(int x, int y)
{
    game_point_t head = game_segment(&game, 0);

    TEST_ASSERT_EQUAL_INT(x, head.x);
    TEST_ASSERT_EQUAL_INT(y, head.y);
}

/* Each segment must touch the one ahead of it: no gaps and no diagonal steps. */
static void expect_body_connected(void)
{
    for (size_t i = 1; i < game.length; i++) {
        game_point_t ahead = game_segment(&game, i - 1);
        game_point_t behind = game_segment(&game, i);

        TEST_ASSERT_EQUAL_INT(1, abs(ahead.x - behind.x) + abs(ahead.y - behind.y));
    }
}

static void test_body_follows_the_head(void)
{
    /* Laps of a 3x3 square, enough to wrap the 240-slot body array twice. The 12-step lap
     * matters: with 14 or 16, a wrong index into the array can land on an identical cell. */
    static const game_direction_t lap[] = {
        GAME_UP,   GAME_UP,   GAME_UP,   GAME_LEFT,  GAME_LEFT,  GAME_LEFT,
        GAME_DOWN, GAME_DOWN, GAME_DOWN, GAME_RIGHT, GAME_RIGHT, GAME_RIGHT,
    };
    const int steps = 40 * 12;

    game_step(&game);
    expect_head(GAME_WIDTH / 2 + 1, GAME_HEIGHT / 2);

    for (int i = 0; i < steps; i++) {
        game_turn(&game, lap[i % 12]);
        game_step(&game);
        expect_body_connected();
    }
    expect_head(GAME_WIDTH / 2 + 1, GAME_HEIGHT / 2);
}

static void test_turns_but_never_reverses_into_its_neck(void)
{
    /* Straight back while moving right: ignored, the snake keeps going right. */
    game_turn(&game, GAME_LEFT);
    game_step(&game);
    expect_head(GAME_WIDTH / 2 + 1, GAME_HEIGHT / 2);

    /* Two keys within one step: up is taken, then left would reverse the last step. */
    game_turn(&game, GAME_UP);
    game_turn(&game, GAME_LEFT);
    game_step(&game);
    expect_head(GAME_WIDTH / 2 + 1, GAME_HEIGHT / 2 - 1);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_body_follows_the_head);
    RUN_TEST(test_turns_but_never_reverses_into_its_neck);
    return UNITY_END();
}
