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
	public:
		virtual std::optional<Token> next() = 0;
		BaseLexerStream() = default;
		BaseLexerStream(const BaseLexerStream&) = delete; // No delete
		BaseLexerStream(const BaseLexerStream&&) = delete; // No Move
	};

	class LexerStream : public BaseLexerStream {
		std::istream& input;

		std::size_t line;
		std::size_t column;
	public:
		explicit LexerStream(std::istream& input) : input(input), line(0), column(0) {}
		std::optional<Token> next() override;
	private:
		inline bool eof() {
			return input.eof();
		}
		char peek() {
			return static_cast<char>(input.peek());
		}
		char get() {
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
		Token readOperator();
		Token readDirective();
		Token readString();
	};
};