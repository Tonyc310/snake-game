#include "keys.h"
#include "unity.h"

#include <string.h>

static keys_decoder_t decoder;

/* Feeds a whole string and returns the last key it produced. */
static keys_input_t decode(const char *bytes)
{
    keys_input_t last = KEYS_NONE;

    for (size_t i = 0; i < strlen(bytes); i++) {
        keys_input_t key = keys_decode(&decoder, (uint8_t)bytes[i]);

        if (key != KEYS_NONE) {
            last = key;
        }
    }
    return last;
}

void setUp(void)
{
    memset(&decoder, 0, sizeof decoder);
}

void tearDown(void)
{
}

static void test_letters_and_arrow_keys_decode(void)
{
    TEST_ASSERT_EQUAL(KEYS_UP, decode("w"));
    TEST_ASSERT_EQUAL(KEYS_LEFT, decode("A"));
    TEST_ASSERT_EQUAL(KEYS_PAUSE, decode("p"));
    TEST_ASSERT_EQUAL(KEYS_RESTART, decode("R"));
    TEST_ASSERT_EQUAL(KEYS_DOWN, decode("\x1b[B"));
    TEST_ASSERT_EQUAL(KEYS_RIGHT, decode("\x1bOC"));
    TEST_ASSERT_EQUAL(KEYS_LEFT, decode("\x1b[1;5D"));
}

static void test_other_input_is_ignored_without_losing_track(void)
{
    TEST_ASSERT_EQUAL(KEYS_NONE, decode("x\x1b[2~"));
    TEST_ASSERT_EQUAL(KEYS_RIGHT, decode("d"));
    TEST_ASSERT_EQUAL(KEYS_UP, decode("\x1bw"));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_letters_and_arrow_keys_decode);
    RUN_TEST(test_other_input_is_ignored_without_losing_track);
    return UNITY_END();
}
