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
			NewLine,
			Comment,
		};
		Token(Type type, std::string text, std::size_t line, std::size_t column) : 
			type(type), text(text), line(line), column(column) {}

        Type type;
        std::string text;

        std::size_t line;
        std::size_t column;

        std::string toString() const {

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
			default:
				typeString = "Unknown";
				break;
            }

            return typeString + "[\"" + text + "\" " + std::to_string(line) + ":" + std::to_string(column) + " ]";
        }

		operator std::string() const { return toString(); }
	};

	class LexerStream {
		std::istream& input;
		std::string buffer;
		char nextChar;

		std::size_t line;
		std::size_t column;
	public:
		LexerStream(std::istream& input) : input(input), line(0), column(0), nextChar(-1), buffer({}) {}
		std::optional<Token> next();
		inline bool eof() {
			return input.eof();
		}
	};

	inline std::vector<Token> process(std::string input) {
		std::istringstream iss(input);   // создаём поток из строки
		std::vector<Token> outputVector{};
		auto stream = LexerStream(iss);

		while (auto next = stream.next()) {
			outputVector.push_back(*next);
		}
		return outputVector;
	}

	inline std::vector<std::string> tokensToStrings(
		const std::vector<Token>& tokens)
	{
		std::vector<std::string> result;

		for (const auto& token : tokens) {
			result.push_back(token);
		}

		return result;
	}
};