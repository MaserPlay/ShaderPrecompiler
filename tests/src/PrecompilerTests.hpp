#include <gtest/gtest.h>

#include "precompiler/precompiler.hpp"

TEST(PrecompilerTests, EmptyInput)
{
    EXPECT_EQ(precompiler::process(""), "");
}

TEST(PrecompilerTests, PreserveSimpleCode)
{
    const std::string input = R"(void main() {
    gl_Position = vec4(1.0);
})";

    EXPECT_EQ(precompiler::process(input), input);
}

TEST(PrecompilerTests, RemoveExtraNewLines)
{
    const std::string input =
        "void main() {\n\n\n"
        "gl_Position = vec4(1.0);\n\n"
        "}\n";

    const std::string expected =
        "void main() {\n"
        "gl_Position = vec4(1.0);\n"
        "}\n";

    EXPECT_EQ(precompiler::process(input), expected);
}

TEST(PrecompilerTests, ProcessDefine)
{
    const std::string input = R"(
#define USE_LIGHT

#ifdef USE_LIGHT
vec3 light = vec3(1.0);
#endif
)";

    const std::string expected = R"(vec3 light = vec3(1.0);
)";

    EXPECT_EQ(precompiler::process(input), expected);
}

TEST(PrecompilerTests, RemoveDisabledIfdef)
{
    const std::string input = R"(
#ifdef UNKNOWN_DEFINE
vec3 light = vec3(1.0);
#endif
)";

    EXPECT_EQ(precompiler::process(input), "");
}

TEST(PrecompilerTests, ProcessIfndef)
{
    const std::string input = R"(
#ifndef DISABLED
float value = 1.0;
#endif
)";

    const std::string expected =
        "float value = 1.0;\n";

    EXPECT_EQ(precompiler::process(input), expected);
}

TEST(PrecompilerTests, ProcessElse)
{
    const std::string input = R"(
#define MOBILE

#ifdef MOBILE
precision mediump float;
#else
precision highp float;
#endif
)";

    const std::string expected =
        "precision mediump float;\n";

    EXPECT_EQ(precompiler::process(input), expected);
}

TEST(PrecompilerTests, NestedIfdefs)
{
    const std::string input = R"(
#define A
#define B

#ifdef A
    #ifdef B
        int value = 1;
    #endif
#endif
)";

    const std::string expected =
        "int value = 1;\n";

    EXPECT_EQ(precompiler::process(input), expected);
}

TEST(PrecompilerTests, UndefMacro)
{
    const std::string input = R"(
#define TEST
#undef TEST

#ifdef TEST
int value = 1;
#endif
)";

    EXPECT_EQ(precompiler::process(input), "");
}

TEST(PrecompilerTests, IgnoreUnknownDirectives)
{
    const std::string input = R"(
#version 330 core

void main() {}
)";

    EXPECT_EQ(precompiler::process(input), input);
}