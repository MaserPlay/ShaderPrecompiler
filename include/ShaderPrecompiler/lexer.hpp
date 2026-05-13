#pragma once

#include <string>
#include <istream>
#include <sstream>
#include <vector>
#include <optional>

namespace shader_precompiler::lexer {
	struct Token
	{
		enum class Type {
			Identifier,   // vec3, color
			Directive,    // #ifdef #define
			Number,       // 1.0
			Operator,     // + - * /
			Symbol,       // ; , ( ) {}
			NewLine,      // \n
			Comment,      //  //
			String        // "text"
		};
		Token(Type type, std::string text, std::size_t line, std::size_t column) : 
			type(type), text(text), line(line), column(column) {}

        Type type;
        std::string text;

        std::size_t line;
        std::size_t column;

        std::string toDebugString() const {

            std::string typeString;
            switch (type) {
            case Type::Identifier:
                typeString = "Identifier";
                break;
			case Type::Directive:
				typeString = "Directive";
                break;
			case Type::Number:
				typeString = "Number";
                break;
			case Type::Operator:
				typeString = "Operator";
                break;
			case Type::Symbol:
				typeString = "Symbol";
                break;
			case Type::NewLine:
				typeString = "NewLine";
                break;
			case Type::Comment:
				typeString = "Comment";
                break;
			case Type::String:
				typeString = "String";
				break;
			default:
				typeString = "Unknown";
				break;
            }

            return typeString + "[\"" + text + "\" " + std::to_string(line) + ":" + std::to_string(column) + " ]";
        }
	};

	class BaseLexerStream {
		std::optional<Token> lookahead{ std::nullopt };
		virtual std::optional<Token> next() = 0;
	public:
		BaseLexerStream() = default;
		BaseLexerStream(const BaseLexerStream&) = delete; // No delete
		BaseLexerStream(const BaseLexerStream&&) = delete; // No Move

		virtual bool eof() = 0;
		std::optional<Token> peek() {
			if (!lookahead.has_value()) {
				lookahead = next();   // прочитать и запомнить
			}
			return lookahead;
		}
		std::optional<Token> get() {
			if (lookahead.has_value()) {
				auto token = std::move(lookahead);
				lookahead.reset();
				return token;
			}
			else {
				return next();
			}
		}
	};

	class LexerStream : public BaseLexerStream {
		std::istream& input;

		std::size_t line;
		std::size_t column;
		std::optional<Token> next() override;
	public:
		explicit LexerStream(std::istream& input) : input(input), line(0), column(0) {}
	private:
		inline bool eof() override {
			return input.eof();
		}
		char peekChar() {
			return static_cast<char>(input.peek());
		}
		char getChar() {
			char c = static_cast<char>(input.get());

			if (c == '\n') {
				line++;
				column = 0;
			}
			else {
				column++;
			}

			return c;
		}
		Token createToken(Token::Type type, std::string text) {
			return Token(type, text, line, column);
		}
		Token readIdentifier();
		Token readNumber(std::string prefix = "");
		Token readSymbol(std::string prefix = "");
		Token readOperator(std::string prefix = "");
		Token readDirective();
		Token readString();
		Token readCommentStartSlash();
	};
};