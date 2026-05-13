#include "../common.hpp"

#include "lexer.hpp"

using namespace shader_precompiler::lexer;

static std::vector<Token> processLexer(std::string input) {
    std::istringstream iss(input);   // создаём поток из строки
    std::vector<Token> outputVector{};
    auto stream = LexerStream(iss);

    while (auto next = stream.get()) {
        outputVector.push_back(*next);
    }
    return outputVector;
}

TEST(Lexer, EmptyInput)
{
    auto tokens = processLexer("");

    EXPECT_TRUE(tokens.empty()) << ::testing::PrintToString(tokensToDebugStrings(tokens));
}
TEST(Lexer, Identifier)
{
    auto tokens = processLexer("hello");

    ASSERT_SIZE(tokens, 1)

    EXPECT_TEXT(tokens[0], "hello")
    EXPECT_TYPE(tokens[0], Token::Type::Identifier)
}

TEST(Lexer, MultipleIdentifiers)
{
    auto tokens = processLexer("hello world");

    ASSERT_SIZE(tokens, 2)

    EXPECT_TEXT(tokens[0], "hello")
    EXPECT_TEXT(tokens[1], "world")

    EXPECT_TYPE(tokens[0], Token::Type::Identifier)
    EXPECT_TYPE(tokens[1], Token::Type::Identifier)
}
TEST(Lexer, Comment)
{
    auto tokens = processLexer("////// texttexttext");

    ASSERT_SIZE(tokens, 1)
}
TEST(Lexer, MultiLineComment)
{
    auto tokens = processLexer("/**** texttexttext\n\n\n\n\n\n\n**************/");

    ASSERT_SIZE(tokens, 1)
}

TEST(Lexer, Number)
{
    auto tokens = processLexer("123");

    ASSERT_SIZE(tokens, 1)

    EXPECT_TEXT(tokens[0], "123")
    EXPECT_TYPE(tokens[0], Token::Type::Number)
}

TEST(Lexer, FloatNumber)
{
    auto tokens = processLexer("1.5");

    ASSERT_SIZE(tokens, 1)

    EXPECT_TEXT(tokens[0], "1.5")
    EXPECT_TYPE(tokens[0], Token::Type::Number)
}

TEST(Lexer, NumberWithMultipleDotsBug)
{
    auto tokens = processLexer("1.52..2.2.2..2.2.2..2");

    ASSERT_SIZE(tokens, 11)

    EXPECT_TEXT(tokens[0], "1.52")
    EXPECT_TYPE(tokens[0], Token::Type::Number)
}

TEST(Lexer, Symbol)
{
    auto tokens = processLexer("{");

    ASSERT_SIZE(tokens, 1)

    EXPECT_TEXT(tokens[0], "{")
    EXPECT_TYPE(tokens[0], Token::Type::Symbol)
}

TEST(Lexer, MultipleSymbols)
{
    std::string sourceString = "{}();";
    auto tokens = processLexer("{}();");

    ASSERT_SIZE(tokens, sourceString.size())

    for (std::size_t i = 0; i < sourceString.size(); i++)
    {
        EXPECT_EQ(tokens[i].text[0], sourceString[i]) << ::testing::PrintToString(tokens[i].toDebugString());
        EXPECT_EQ(tokens[i].type, Token::Type::Symbol) << ::testing::PrintToString(tokens[i].toDebugString());
    }
}

TEST(Lexer, Operator)
{
    auto tokens = processLexer("=");

    ASSERT_SIZE(tokens, 1)

    EXPECT_TEXT(tokens[0], "=")
    EXPECT_TYPE(tokens[0], Token::Type::Operator)
}

TEST(Lexer, ComplexExpression)
{
    auto tokens = processLexer("vec3 color = vec3(1.0);");

    ASSERT_EQ(tokens.size(), 8) << ::testing::PrintToString(tokensToDebugStrings(tokens));

    EXPECT_TEXT(tokens[0], "vec3")
    EXPECT_TEXT(tokens[1], "color")
    EXPECT_TEXT(tokens[2], "=")
    EXPECT_TEXT(tokens[3], "vec3")
    EXPECT_TEXT(tokens[4], "(")
    EXPECT_TEXT(tokens[5], "1.0")
    EXPECT_TEXT(tokens[6], ")")
    EXPECT_TEXT(tokens[7], ";")

    EXPECT_TYPE(tokens[0], Token::Type::Identifier)
    EXPECT_TYPE(tokens[1], Token::Type::Identifier)
    EXPECT_TYPE(tokens[2], Token::Type::Operator)
    EXPECT_TYPE(tokens[3], Token::Type::Identifier)
    EXPECT_TYPE(tokens[4], Token::Type::Symbol)
    EXPECT_TYPE(tokens[5], Token::Type::Number)
    EXPECT_TYPE(tokens[6], Token::Type::Symbol)
    EXPECT_TYPE(tokens[7], Token::Type::Symbol)
}

TEST(Lexer, Directive)
{
    auto tokens = processLexer("#ifdef");

    ASSERT_SIZE(tokens, 1)

    EXPECT_TEXT(tokens[0], "#ifdef")
    EXPECT_TYPE(tokens[0], Token::Type::Directive)
}

TEST(Lexer, IfdefBlock)
{
    auto tokens = processLexer(
        "#ifdef LIGHTING\n"
        "vec3 color;\n"
        "#endif"
    );

    ASSERT_SIZE(tokens, 8)

    EXPECT_TEXT(tokens[0], "#ifdef")
    EXPECT_TEXT(tokens[1], "LIGHTING")

    EXPECT_TYPE(tokens[0], Token::Type::Directive)
    EXPECT_TYPE(tokens[1], Token::Type::Identifier)
}

TEST(Lexer, IgnoreWhitespace)
{
    auto tokens = processLexer("   hello     world   ");

    ASSERT_SIZE(tokens, 2)

    EXPECT_TEXT(tokens[0], "hello")
    EXPECT_TEXT(tokens[1], "world")
}

TEST(Lexer, LineAndColumn)
{
    auto tokens = processLexer(
        "abc\n"
        "def"
    );

    ASSERT_EQ(tokens.size(), 3) << ::testing::PrintToString(tokensToDebugStrings(tokens));

    EXPECT_EQ(tokens[0].line, 0) << ::testing::PrintToString(tokens[0].toDebugString());
    EXPECT_EQ(tokens[0].column, 0) << ::testing::PrintToString(tokens[0].toDebugString());

    EXPECT_EQ(tokens[2].line, 1) << ::testing::PrintToString(tokens[2].toDebugString());
}

TEST(Lexer, ShaderExample)
{
    auto tokens = processLexer(
        "#version 330 core\n"
        "layout(location = 0) in vec3 aPos;"
    );

    ASSERT_SIZE(tokens, 14)

    EXPECT_TEXT(tokens[0], "#version")
    EXPECT_TYPE(tokens[0], Token::Type::Directive)
}

TEST(Lexer, FloatNumberStartDot)
{
    auto tokens = processLexer(
        "double a = .1;"
    );

    ASSERT_SIZE(tokens, 5)

    EXPECT_TEXT(tokens[0], "double")
    EXPECT_TYPE(tokens[0], Token::Type::Identifier)

    EXPECT_TEXT(tokens[1], "a")
    EXPECT_TYPE(tokens[1], Token::Type::Identifier)

    EXPECT_TEXT(tokens[2], "=")
    EXPECT_TYPE(tokens[2], Token::Type::Operator)

    EXPECT_TEXT(tokens[3], ".1")
    EXPECT_TYPE(tokens[3], Token::Type::Number)

    EXPECT_TEXT(tokens[4], ";")
    EXPECT_TYPE(tokens[4], Token::Type::Symbol)
}

TEST(Lexer, FunctionCallWithClassAndArguments)
{
    auto tokens = processLexer(
        "double a = class_name.func_name(.1, \"string\", property);"
    );
    
    ASSERT_SIZE(tokens, 14)

    EXPECT_TEXT(tokens[0], "double")
    EXPECT_TEXT(tokens[1], "a")
    EXPECT_TEXT(tokens[2], "=")
    EXPECT_TEXT(tokens[3], "class_name")
    EXPECT_TEXT(tokens[4], ".")
    EXPECT_TEXT(tokens[5], "func_name")
    EXPECT_TEXT(tokens[6], "(")
    EXPECT_TEXT(tokens[7], ".1")
    EXPECT_TEXT(tokens[8], ",")
    EXPECT_TEXT(tokens[9], "\"string\"")
    EXPECT_TEXT(tokens[10], ",")
    EXPECT_TEXT(tokens[11], "property")
    EXPECT_TEXT(tokens[12], ")")
    EXPECT_TEXT(tokens[13], ";")

    EXPECT_TYPE(tokens[0], Token::Type::Identifier)
    EXPECT_TYPE(tokens[1], Token::Type::Identifier)
    EXPECT_TYPE(tokens[2], Token::Type::Operator)
    EXPECT_TYPE(tokens[3], Token::Type::Identifier)
    EXPECT_TYPE(tokens[4], Token::Type::Symbol)
    EXPECT_TYPE(tokens[5], Token::Type::Identifier)
    EXPECT_TYPE(tokens[6], Token::Type::Symbol)
    EXPECT_TYPE(tokens[7], Token::Type::Number)
    EXPECT_TYPE(tokens[8], Token::Type::Symbol)
    EXPECT_TYPE(tokens[9], Token::Type::String)
    EXPECT_TYPE(tokens[10], Token::Type::Symbol)
    EXPECT_TYPE(tokens[11], Token::Type::Identifier)
    EXPECT_TYPE(tokens[12], Token::Type::Symbol)
    EXPECT_TYPE(tokens[13], Token::Type::Symbol)
}