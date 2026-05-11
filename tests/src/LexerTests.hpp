#include <gtest/gtest.h>

#include "lexer.hpp"

using namespace shader_precompiler::lexer;

TEST(Lexer, EmptyInput)
{
    auto tokens = process("");

    EXPECT_TRUE(tokens.empty()) << ::testing::PrintToString(tokensToStrings(tokens));
}
TEST(Lexer, Identifier)
{
    auto tokens = process("hello");

    ASSERT_EQ(tokens.size(), 1) << ::testing::PrintToString(tokensToStrings(tokens));

    EXPECT_EQ(tokens[0].text, "hello") << ::testing::PrintToString(tokens[0].toString());
    EXPECT_EQ(tokens[0].type, Token::Type::Identifier) << ::testing::PrintToString(tokens[0].toString());
}

TEST(Lexer, MultipleIdentifiers)
{
    auto tokens = process("hello world");

    ASSERT_EQ(tokens.size(), 2) << ::testing::PrintToString(tokensToStrings(tokens));

    EXPECT_EQ(tokens[0].text, "hello") << ::testing::PrintToString(tokens[0].toString());
    EXPECT_EQ(tokens[1].text, "world") << ::testing::PrintToString(tokens[1].toString());

    EXPECT_EQ(tokens[0].type, Token::Type::Identifier) << ::testing::PrintToString(tokens[0].toString());
    EXPECT_EQ(tokens[1].type, Token::Type::Identifier) << ::testing::PrintToString(tokens[1].toString());
}

TEST(Lexer, Number)
{
    auto tokens = process("123");

    ASSERT_EQ(tokens.size(), 1) << ::testing::PrintToString(tokensToStrings(tokens));

    EXPECT_EQ(tokens[0].text, "123") << ::testing::PrintToString(tokens[0].toString());
    EXPECT_EQ(tokens[0].type, Token::Type::Number) << ::testing::PrintToString(tokens[0].toString());
}

TEST(Lexer, FloatNumber)
{
    auto tokens = process("1.5");

    ASSERT_EQ(tokens.size(), 1) << ::testing::PrintToString(tokensToStrings(tokens));

    EXPECT_EQ(tokens[0].text, "1.5") << ::testing::PrintToString(tokens[0].toString());
    EXPECT_EQ(tokens[0].type, Token::Type::Number) << ::testing::PrintToString(tokens[0].toString());
}

TEST(Lexer, Symbol)
{
    auto tokens = process("{");

    ASSERT_EQ(tokens.size(), 1) << ::testing::PrintToString(tokensToStrings(tokens));

    EXPECT_EQ(tokens[0].text, "{") << ::testing::PrintToString(tokens[0].toString());
    EXPECT_EQ(tokens[0].type, Token::Type::Symbol) << ::testing::PrintToString(tokens[0].toString());
}

TEST(Lexer, MultipleSymbols)
{
    std::string sourceString = "{}();";
    auto tokens = process("{}();");

    ASSERT_EQ(tokens.size(), sourceString.size()) << ::testing::PrintToString(tokensToStrings(tokens));

    for (std::size_t i = 0; i < sourceString.size(); i++)
    {
        EXPECT_EQ(tokens[i].text[0], sourceString[i]) << ::testing::PrintToString(tokens[i].toString());
        EXPECT_EQ(tokens[i].type, Token::Type::Symbol) << ::testing::PrintToString(tokens[i].toString());
    }
}

TEST(Lexer, Operator)
{
    auto tokens = process("=");

    ASSERT_EQ(tokens.size(), 1) << ::testing::PrintToString(tokensToStrings(tokens));

    EXPECT_EQ(tokens[0].text, "=") << ::testing::PrintToString(tokens[0].toString());
    EXPECT_EQ(tokens[0].type, Token::Type::Operator) << ::testing::PrintToString(tokens[0].toString());
}

TEST(Lexer, ComplexExpression)
{
    auto tokens = process("vec3 color = vec3(1.0);");

    ASSERT_EQ(tokens.size(), 8) << ::testing::PrintToString(tokensToStrings(tokens));

    EXPECT_EQ(tokens[0].text, "vec3") << ::testing::PrintToString(tokens[0].toString());
    EXPECT_EQ(tokens[1].text, "color") << ::testing::PrintToString(tokens[1].toString());
    EXPECT_EQ(tokens[2].text, "=") << ::testing::PrintToString(tokens[2].toString());
    EXPECT_EQ(tokens[3].text, "vec3") << ::testing::PrintToString(tokens[3].toString());
    EXPECT_EQ(tokens[4].text, "(") << ::testing::PrintToString(tokens[4].toString());
    EXPECT_EQ(tokens[5].text, "1.0") << ::testing::PrintToString(tokens[5].toString());
    EXPECT_EQ(tokens[6].text, ")") << ::testing::PrintToString(tokens[6].toString());
    EXPECT_EQ(tokens[7].text, ";") << ::testing::PrintToString(tokens[7].toString());
}

TEST(Lexer, Directive)
{
    auto tokens = process("#ifdef");

    ASSERT_EQ(tokens.size(), 1) << ::testing::PrintToString(tokensToStrings(tokens));

    EXPECT_EQ(tokens[0].text, "#ifdef") << ::testing::PrintToString(tokens[0].toString());
    EXPECT_EQ(tokens[0].type, Token::Type::Directive) << ::testing::PrintToString(tokens[0].toString());
}

TEST(Lexer, IfdefBlock)
{
    auto tokens = process(
        "#ifdef LIGHTING\n"
        "vec3 color;\n"
        "#endif"
    );

    ASSERT_GE(tokens.size(), 4) << ::testing::PrintToString(tokensToStrings(tokens));

    EXPECT_EQ(tokens[0].text, "#ifdef") << ::testing::PrintToString(tokens[0].toString());
    EXPECT_EQ(tokens[1].text, "LIGHTING") << ::testing::PrintToString(tokens[1].toString());

    EXPECT_EQ(tokens[0].type, Token::Type::Directive) << ::testing::PrintToString(tokens[0].toString());
    EXPECT_EQ(tokens[1].type, Token::Type::Identifier) << ::testing::PrintToString(tokens[1].toString());
}

TEST(Lexer, IgnoreWhitespace)
{
    auto tokens = process("   hello     world   ");

    ASSERT_EQ(tokens.size(), 2) << ::testing::PrintToString(tokensToStrings(tokens));

    EXPECT_EQ(tokens[0].text, "hello") << ::testing::PrintToString(tokens[0].toString());
    EXPECT_EQ(tokens[1].text, "world") << ::testing::PrintToString(tokens[1].toString());
}

TEST(Lexer, LineAndColumn)
{
    auto tokens = process(
        "abc\n"
        "def"
    );

    ASSERT_EQ(tokens.size(), 3) << ::testing::PrintToString(tokensToStrings(tokens));

    EXPECT_EQ(tokens[0].line, 0) << ::testing::PrintToString(tokens[0].toString());
    EXPECT_EQ(tokens[0].column, 0) << ::testing::PrintToString(tokens[0].toString());

    EXPECT_EQ(tokens[2].line, 1) << ::testing::PrintToString(tokens[2].toString());
}

TEST(Lexer, ShaderExample)
{
    auto tokens = process(
        "#version 330 core\n"
        "layout(location = 0) in vec3 aPos;"
    );

    ASSERT_FALSE(tokens.empty()) << ::testing::PrintToString(tokensToStrings(tokens));

    EXPECT_EQ(tokens[0].text, "#version") << ::testing::PrintToString(tokens[0].toString());
    EXPECT_EQ(tokens[0].type, Token::Type::Directive) << ::testing::PrintToString(tokens[0].toString());
}

TEST(Lexer, FloatNumberStartDot)
{
    auto tokens = process(
        "double a = .1;"
    );

    ASSERT_EQ(tokens.size(), 5) << ::testing::PrintToString(tokensToStrings(tokens));

    EXPECT_EQ(tokens[0].text, "double") << ::testing::PrintToString(tokens[0].toString());
    EXPECT_EQ(tokens[0].type, Token::Type::Identifier) << ::testing::PrintToString(tokens[0].toString());

    EXPECT_EQ(tokens[1].text, "a") << ::testing::PrintToString(tokens[1].toString());
    EXPECT_EQ(tokens[1].type, Token::Type::Identifier) << ::testing::PrintToString(tokens[1].toString());

    EXPECT_EQ(tokens[2].text, "=") << ::testing::PrintToString(tokens[2].toString());
    EXPECT_EQ(tokens[2].type, Token::Type::Operator) << ::testing::PrintToString(tokens[2].toString());

    EXPECT_EQ(tokens[3].text, ".1") << ::testing::PrintToString(tokens[3].toString());
    EXPECT_EQ(tokens[3].type, Token::Type::Number) << ::testing::PrintToString(tokens[3].toString());

    EXPECT_EQ(tokens[4].text, ";") << ::testing::PrintToString(tokens[4].toString());
    EXPECT_EQ(tokens[4].type, Token::Type::Symbol) << ::testing::PrintToString(tokens[4].toString());
}

TEST(Lexer, FunctionCallWithClassAndArguments)
{
    auto tokens = process(
        "double a = class_name.func_name(.1, \"string\", property);"
    );

    ASSERT_EQ(tokens.size(), 14)
        << ::testing::PrintToString(tokensToStrings(tokens));

    EXPECT_EQ(tokens[0].text, "double")
        << ::testing::PrintToString(tokens[0].toString());
    EXPECT_EQ(tokens[0].type, Token::Type::Identifier)
        << ::testing::PrintToString(tokens[0].toString());

    EXPECT_EQ(tokens[1].text, "a")
        << ::testing::PrintToString(tokens[1].toString());
    EXPECT_EQ(tokens[1].type, Token::Type::Identifier)
        << ::testing::PrintToString(tokens[1].toString());

    EXPECT_EQ(tokens[2].text, "=")
        << ::testing::PrintToString(tokens[2].toString());
    EXPECT_EQ(tokens[2].type, Token::Type::Operator)
        << ::testing::PrintToString(tokens[2].toString());

    EXPECT_EQ(tokens[3].text, "class_name")
        << ::testing::PrintToString(tokens[3].toString());
    EXPECT_EQ(tokens[3].type, Token::Type::Identifier)
        << ::testing::PrintToString(tokens[3].toString());

    EXPECT_EQ(tokens[4].text, ".")
        << ::testing::PrintToString(tokens[4].toString());
    EXPECT_EQ(tokens[4].type, Token::Type::Symbol)
        << ::testing::PrintToString(tokens[4].toString());

    EXPECT_EQ(tokens[5].text, "func_name")
        << ::testing::PrintToString(tokens[5].toString());
    EXPECT_EQ(tokens[5].type, Token::Type::Identifier)
        << ::testing::PrintToString(tokens[5].toString());

    EXPECT_EQ(tokens[6].text, "(")
        << ::testing::PrintToString(tokens[6].toString());
    EXPECT_EQ(tokens[6].type, Token::Type::Symbol)
        << ::testing::PrintToString(tokens[6].toString());

    EXPECT_EQ(tokens[7].text, ".1")
        << ::testing::PrintToString(tokens[7].toString());
    EXPECT_EQ(tokens[7].type, Token::Type::Number)
        << ::testing::PrintToString(tokens[7].toString());

    EXPECT_EQ(tokens[8].text, ",")
        << ::testing::PrintToString(tokens[8].toString());
    EXPECT_EQ(tokens[8].type, Token::Type::Symbol)
        << ::testing::PrintToString(tokens[8].toString());

    EXPECT_EQ(tokens[9].text, "string")
        << ::testing::PrintToString(tokens[9].toString());

    EXPECT_EQ(tokens[10].text, ",")
        << ::testing::PrintToString(tokens[10].toString());
    EXPECT_EQ(tokens[10].type, Token::Type::Symbol)
        << ::testing::PrintToString(tokens[10].toString());

    EXPECT_EQ(tokens[11].text, "property")
        << ::testing::PrintToString(tokens[11].toString());
    EXPECT_EQ(tokens[11].type, Token::Type::Identifier)
        << ::testing::PrintToString(tokens[11].toString());

    EXPECT_EQ(tokens[12].text, ")")
        << ::testing::PrintToString(tokens[12].toString());
    EXPECT_EQ(tokens[12].type, Token::Type::Symbol)
        << ::testing::PrintToString(tokens[12].toString());

    EXPECT_EQ(tokens[13].text, ";")
        << ::testing::PrintToString(tokens[13].toString());
    EXPECT_EQ(tokens[13].type, Token::Type::Symbol)
        << ::testing::PrintToString(tokens[13].toString());
}