#include <cassert>
#include <iostream>
#include <string>

#include <gtest/gtest.h>

#include <search_engine/header.hpp>

TEST(TokenizatorTest, WcharToChar) {
    ASSERT_EQ(wchar_to_char('Q'), L'q');
}
