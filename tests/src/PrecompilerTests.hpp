#include "../common.hpp"

#include <vector>

#include "precompiler.hpp"

static std::vector<shader_precompiler::lexer::Token> processPrecompiler(std::string base) {
	std::istringstream iss(base);   // создаём поток из строки

	shader_precompiler::lexer::LexerStream tokenStream{ iss };

	shader_precompiler::precompiler::PrecompilerLexerStream afterPreprocessor{ tokenStream };

	std::vector<shader_precompiler::lexer::Token> outputVector{};

	while (auto next = afterPreprocessor.get()) {
		outputVector.push_back(*next);
	}
	return outputVector;
}

TEST(PrecompilerTests, EmptyInput)
{
	auto tokens = processPrecompiler("");
	ASSERT_SIZE(tokens, 0)
}

TEST(PrecompilerTests, SimplyExpression) {
	auto tokens = processPrecompiler("a = 2");

	ASSERT_SIZE(tokens, 3)

	EXPECT_TEXT(tokens[0], "a")
	EXPECT_TEXT(tokens[1], "=")
	EXPECT_TEXT(tokens[2], "2")

	EXPECT_TYPE(tokens[0], Token::Type::Identifier)
	EXPECT_TYPE(tokens[1], Token::Type::Operator)
	EXPECT_TYPE(tokens[2], Token::Type::Number)
}

TEST(PrecompilerTests, PreserveSimpleCode)
{
	auto tokens = processPrecompiler("void main() {//tetettet\ngl_Position = vec4(1.0);\n}");
	ASSERT_SIZE(tokens, 13)
}

TEST(PrecompilerTests, RemoveExtraNewLines)
{
	auto tokens = processPrecompiler("void main() {\n\n\n"
		"gl_Position = vec4(1.0);\n\n"
		"}\n");

	ASSERT_SIZE(tokens, 13)
}

TEST(PrecompilerTests, ProcessDefine)
{
	auto tokens = processPrecompiler(R"(
#define USE_LIGHT

#ifdef USE_LIGHT
vec3 light = vec3(1.0);
#endif
)");

	ASSERT_SIZE(tokens, 8)

	EXPECT_TEXT(tokens[0], "vec3")
	EXPECT_TEXT(tokens[1], "light")
	EXPECT_TEXT(tokens[2], "=")
	EXPECT_TEXT(tokens[3], "vec3")
	EXPECT_TEXT(tokens[4], "(")
	EXPECT_TEXT(tokens[5], "1.0")
	EXPECT_TEXT(tokens[6], ")")
	EXPECT_TEXT(tokens[7], ";")
}

TEST(PrecompilerTests, RemoveDisabledIfdef)
{
	auto tokens = processPrecompiler(R"(
#ifdef UNKNOWN_DEFINE
vec3 light = vec3(1.0);
#endif
)");

	ASSERT_SIZE(tokens, 0)
}

TEST(PrecompilerTests, ProcessIfndef)
{
	auto tokens = processPrecompiler(R"(
#ifndef DISABLED
float value = 1.0;
#endif
)");

	ASSERT_SIZE(tokens, 5)

	EXPECT_TEXT(tokens[0], "float")
	EXPECT_TEXT(tokens[1], "value")
	EXPECT_TEXT(tokens[2], "=")
	EXPECT_TEXT(tokens[3], "1.0")
	EXPECT_TEXT(tokens[4], ";")
}

TEST(PrecompilerTests, ProcessElse)
{
	auto tokens = processPrecompiler(R"(
#define MOBILE

#ifdef MOBILE
precision mediump float;
#else
precision highp float;
#endif
)");

	ASSERT_SIZE(tokens, 4)

	EXPECT_TEXT(tokens[0], "precision")
	EXPECT_TEXT(tokens[1], "mediump")
	EXPECT_TEXT(tokens[2], "float")
}

TEST(PrecompilerTests, NestedIfdefs)
{
	auto tokens = processPrecompiler(R"(
#define A
#define B

#ifdef A
    #ifdef B
        int value = 1;
    #endif
#endif
)");

	ASSERT_SIZE(tokens, 5)

	EXPECT_TEXT(tokens[0], "int")
	EXPECT_TEXT(tokens[1], "value")
	EXPECT_TEXT(tokens[2], "=")
	EXPECT_TEXT(tokens[3], "1")
	EXPECT_TEXT(tokens[4], ";")
}

TEST(PrecompilerTests, UndefMacro)
{
	auto tokens = processPrecompiler(R"(
#define TEST
#undef TEST

#ifdef TEST
int value = 1;
#endif
)");

	ASSERT_SIZE(tokens, 0)
}

TEST(PrecompilerTests, DefineReplaceWork)
{
	auto tokens = processPrecompiler(R"(
#define TEST text 2 3
TEST
#warning warning works!!!
#pragma
)");

	ASSERT_SIZE(tokens, 3)

	EXPECT_TEXT(tokens[0], "text")
	EXPECT_TYPE(tokens[0], shader_precompiler::lexer::Token::Type::Identifier)
}

TEST(PrecompilerTests, ReplaceNestedDefines)
{
	auto tokens = processPrecompiler(R"(
#define TEST text
#define TEST_TWO TEST
TEST_TWO
)");

	ASSERT_SIZE(tokens, 1)

	EXPECT_TEXT(tokens[0], "text")
	EXPECT_TYPE(tokens[0], shader_precompiler::lexer::Token::Type::Identifier)
}

TEST(PrecompilerTests, RightDefineReplaceWork)
{
	auto tokens = processPrecompiler(R"(
#define MAX 100
int MAXIMUM = MAX;
)");

	ASSERT_SIZE(tokens, 5)

	EXPECT_TEXT(tokens[0], "int")
	EXPECT_TEXT(tokens[1], "MAXIMUM")
	EXPECT_TEXT(tokens[2], "=")
	EXPECT_TEXT(tokens[3], "100")
	EXPECT_TEXT(tokens[4], ";")
}

TEST(PrecompilerTests, NestedFalseTrueIfdefs)
{
	auto tokens = processPrecompiler(R"(
#define TEST

#ifndef NOT_FOUND
#ifndef TEST
text
#endif
#endif
)");

	ASSERT_SIZE(tokens, 0)
}

TEST(PrecompilerTests, NestedTrueFalseIfdefs)
{
	auto tokens = processPrecompiler(R"(
#define TEST

#ifndef TEST
#ifndef NOT_FOUND
text
#endif
#endif
)");

	ASSERT_SIZE(tokens, 0)
}