#pragma once

#include <string>
#include <istream>
#include <sstream>
#include <vector>
#include <optional>

#include "shader_precompiler.hpp"

namespace shader_precompiler::lexer {
	struct Token
	{
		enum class Type {
			/// <summary>
			/// vec3, color
			/// </summary>
			Identifier,
			/// <summary>
			/// #ifdef #define
			/// </summary>
			Directive,
			/// <summary>
			/// 1.0 1 2 101010
			/// </summary>
			Number,
			/// <summary>
			/// + - * / = % & ^ | ! ?
			/// </summary>
			Operator,
			/// <summary>
			/// ; , ( ) { } "
			/// </summary>
			Symbol,
			/// <summary>
			/// \n
			/// </summary>
			NewLine,
			/// <summary>
			/// // /* */
			/// </summary>
			Comment,
			/// <summary>
			/// "text" "aksasfuyasg"
			/// </summary>
			String
		};
		Token(Type type, std::string text, std::size_t line, std::size_t column) :
			type(type), text(text), location(Location(line, column)) {}

		Token(Type type, std::string text) :
			Token(type, text, 0, 0) {}

		Type type;
		std::string text;
		Location location;

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

			return typeString + "[\"" + text + "\" " + location.toString() + " ]";
		}
		bool operator==(const Token& token) const {
			return type == token.type && text == token.text;
		}
		bool operator!=(const Token& token) const {
			return !(token == (*this));
		}
	};

	class BaseLexerStream {
		std::optional<Token> lookahead{ std::nullopt };
		virtual std::optional<Token> next() = 0;

		static bool saveToStream$needSpaceBetween(Token::Type t) {
			return t == Token::Type::Identifier ||
				t == Token::Type::Number;
		}
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

		inline void saveToStream(std::ostream& stream) {
			Token::Type lastType = (Token::Type)-1;
			while (auto next = this->get()) {
				if (saveToStream$needSpaceBetween(lastType) && saveToStream$needSpaceBetween(next->type)) {
					stream << ' ';
				}
				stream << next->text;
				lastType = next->type;
			}
		}
	};

	class LexerStream : public BaseLexerStream {
		std::istream& input;

		std::size_t line;
		std::size_t column;
		std::optional<Token> next() override;

		IDiagnosticReporter& reporter;

		PRINT_ERROR_DEFINE(shader_precompiler::Error::Stage::LEXER)
	public:
		explicit LexerStream(std::istream& input, IDiagnosticReporter& reporter) : input(input), line(0), column(0), reporter(reporter) {}
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