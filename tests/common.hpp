#pragma once

#include <gtest/gtest.h>

#include "lexer.hpp"

inline std::vector<std::string> tokensToDebugStrings(
    const std::vector<shader_precompiler::lexer::Token>& tokens)
{
    std::vector<std::string> result;

    for (const auto& token : tokens) {
        result.push_back(token.toDebugString());
    }

    return result;
}

#define ASSERT_SIZE(base, target) ASSERT_EQ(base.size(), target) << ::testing::PrintToString(tokensToDebugStrings(base));
#define EXPECT_TEXT(base, target) EXPECT_EQ(base.text, target) << ::testing::PrintToString(base.toDebugString());
#define EXPECT_TYPE(base, target) EXPECT_EQ(base.type, target) << ::testing::PrintToString(base.toDebugString());