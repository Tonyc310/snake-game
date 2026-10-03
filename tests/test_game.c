#include "game.h"
#include "unity.h"

#include <stdlib.h>

#define SEED 1u

static game_t game;

void setUp(void)
{
    game_init(&game, SEED);
}

void tearDown(void)
{
}

static void expect_cell(game_point_t expected, game_point_t actual)
{
    TEST_ASSERT_EQUAL_INT(expected.x, actual.x);
    TEST_ASSERT_EQUAL_INT(expected.y, actual.y);
}

static void expect_head(int x, int y)
{
    expect_cell((game_point_t){x, y}, game_segment(&game, 0));
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

static void expect_food_off_snake(void)
{
    for (size_t i = 0; i < game.length; i++) {
        game_point_t segment = game_segment(&game, i);

        TEST_ASSERT_FALSE(segment.x == game.food.x && segment.y == game.food.y);
    }
}

/* Puts food on the cell to the right of the head and steps onto it, then switches food off. */
static void eat_to_the_right(void)
{
    game_point_t head = game_segment(&game, 0);

    game.food = (game_point_t){head.x + 1, head.y};
    game.has_food = true;
    game_step(&game);
    game.has_food = false;
}

/* Cell n of a path that runs back and forth along the rows, covering the whole board. */
static game_point_t serpentine(int n)
{
    int y = n / GAME_WIDTH;
    int x = n % GAME_WIDTH;

    return (game_point_t){y % 2 == 0 ? x : GAME_WIDTH - 1 - x, y};
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

    game.has_food = false; /* so the snake can't grow into itself mid-lap */
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

static void test_eating_grows_the_snake_and_scores(void)
{
    const game_point_t ahead = {GAME_WIDTH / 2 + 1, GAME_HEIGHT / 2};
    const game_point_t tail = game_segment(&game, game.length - 1);
    game_t replay;

    /* The same seed must replay the same food, before and after eating. */
    game_init(&replay, SEED);
    expect_cell(replay.food, game.food);
    game.food = ahead;
    replay.food = ahead;

    game_step(&game);
    game_step(&replay);

    TEST_ASSERT_EQUAL_size_t(4, game.length);
    TEST_ASSERT_EQUAL_UINT(1, game.score);
    expect_cell(tail, game_segment(&game, game.length - 1));
    expect_food_off_snake();
    expect_cell(replay.food, game.food);

    /* A zero seed must still give a working generator, not one stuck at zero. */
    game_init(&replay, 0);
    TEST_ASSERT_NOT_EQUAL_UINT32(0, replay.rng);
}

static void test_running_into_a_wall_ends_the_game(void)
{
    game.has_food = false;
    for (int x = GAME_WIDTH / 2 + 1; x < GAME_WIDTH; x++) {
        game_step(&game);
    }
    expect_head(GAME_WIDTH - 1, GAME_HEIGHT / 2);
    TEST_ASSERT_EQUAL(GAME_PLAYING, game.status);

    game_step(&game);
    TEST_ASSERT_EQUAL(GAME_OVER, game.status);
    expect_head(GAME_WIDTH - 1, GAME_HEIGHT / 2); /* stopped where it hit */
}

static void test_chasing_the_tail_is_safe_but_biting_the_body_is_not(void)
{
    /* Circling a 2x2 square steps into the cell the tail is leaving, until the snake is longer. */
    static const game_direction_t loop[] = {GAME_DOWN, GAME_LEFT, GAME_UP, GAME_RIGHT};

    game.has_food = false;
    eat_to_the_right(); /* length 4 */
    for (int i = 0; i < 8; i++) {
        game_turn(&game, loop[i % 4]);
        game_step(&game);
    }
    TEST_ASSERT_EQUAL(GAME_PLAYING, game.status);

    eat_to_the_right(); /* length 5 */
    for (int i = 0; i < 3; i++) {
        game_turn(&game, loop[i]);
        game_step(&game);
    }
    TEST_ASSERT_EQUAL(GAME_OVER, game.status);

    /* A finished game stays finished, even when the next move would be legal. */
    game_turn(&game, GAME_LEFT);
    game_step(&game);
    TEST_ASSERT_EQUAL(GAME_OVER, game.status);
    expect_head(GAME_WIDTH / 2 + 1, GAME_HEIGHT / 2 + 1);
}

static void test_food_only_lands_on_free_cells_and_a_full_board_wins(void)
{
    /* Lay the snake along the serpentine with two cells left: food right ahead, then one more. */
    for (int n = 0; n < GAME_MAX_LENGTH - 2; n++) {
        game.body[n] = serpentine(n);
    }
    game.head = GAME_MAX_LENGTH - 3;
    game.length = GAME_MAX_LENGTH - 2;
    game.moving = GAME_LEFT;
    game.heading = GAME_LEFT;
    game.food = serpentine(GAME_MAX_LENGTH - 2);

    game_step(&game);
    TEST_ASSERT_TRUE(game.has_food);
    expect_cell(serpentine(GAME_MAX_LENGTH - 1), game.food);

    game_step(&game);
    TEST_ASSERT_EQUAL_size_t(GAME_MAX_LENGTH, game.length);
    TEST_ASSERT_FALSE(game.has_food);
    TEST_ASSERT_EQUAL(GAME_WON, game.status);
    expect_body_connected();
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_body_follows_the_head);
    RUN_TEST(test_turns_but_never_reverses_into_its_neck);
    RUN_TEST(test_eating_grows_the_snake_and_scores);
    RUN_TEST(test_running_into_a_wall_ends_the_game);
    RUN_TEST(test_chasing_the_tail_is_safe_but_biting_the_body_is_not);
    RUN_TEST(test_food_only_lands_on_free_cells_and_a_full_board_wins);
    return UNITY_END();
}
