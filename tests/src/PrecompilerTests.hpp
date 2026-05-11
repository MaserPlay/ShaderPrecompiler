#include <gtest/gtest.h>

#include "precompiler.hpp"

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
		"}";

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

	const std::string expected = R"(vec3 light = vec3(1.0);)";

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
		"float value = 1.0;";

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
		"precision mediump float;";

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
		"        int value = 1;";

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

TEST(PrecompilerTests, defineReplaceWork)
{
	const std::string input = R"(
#define TEST text
TEST
#warning warning works!!!
#pragma wtf?
)";
	EXPECT_EQ(precompiler::process(input), "text");
}

TEST(PrecompilerTests, replaceNestedDefines)
{
	const std::string input = R"(
#define TEST text
#define TEST_TWO TEST
TEST_TWO
)";
	EXPECT_EQ(precompiler::process(input), "text");
}

TEST(PrecompilerTests, rightDefineReplaceWork)
{
	const std::string input = R"(
#define MAX 100
int MAXIMUM = MAX;
)";
	EXPECT_EQ(precompiler::process(input), "int MAXIMUM = 100;");
}

TEST(PrecompilerTests, nestedFalseTrueIfdefs)
{
	const std::string input = R"(
#define TEST

#ifndef NOT_FOUND
#ifndef TEST
text
#endif
#endif
)";

	EXPECT_EQ(precompiler::process(input), "");
}
TEST(PrecompilerTests, nestedTrueFalseIfdefs)
{
	const std::string input = R"(
#define TEST

#ifndef TEST
#ifndef NOT_FOUND
text
#endif
#endif
)";

	EXPECT_EQ(precompiler::process(input), "");
}