#include "format.h"
#include "unity.h"

#include <stdint.h>

void setUp(void)
{
}

void tearDown(void)
{
}

static void test_formats_zero_through_the_largest_value(void)
{
    char text[FORMAT_UINT_SIZE];

    TEST_ASSERT_EQUAL(1, format_uint(text, 0u));
    TEST_ASSERT_EQUAL_STRING("0", text);
    TEST_ASSERT_EQUAL(3, format_uint(text, 237u));
    TEST_ASSERT_EQUAL_STRING("237", text);
    TEST_ASSERT_EQUAL(10, format_uint(text, UINT32_MAX));
    TEST_ASSERT_EQUAL_STRING("4294967295", text);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_formats_zero_through_the_largest_value);
    return UNITY_END();
}
