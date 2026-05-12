#include "lexer.hpp"
#include "print_error.hpp"

#include <cctype>

enum class ErrorCodes {
	UNEXPECTED_TOKEN
};

static void printError(ErrorCodes code, std::string text, std::size_t line, std::size_t column) {
	shader_precompiler::setError(shader_precompiler::Error{
		shader_precompiler::Error::Stage::LEXER, (std::size_t)code, text, line, column
		});
}

bool isOperator(char sym) {
	for (char this_char : "+-*/=%&^|!?") {
		if (sym == this_char) {
			return true;
		}
	}
	return false;
}

bool isSymbol(char sym) {
	for (char this_char : ";,(){}\"") {
		if (sym == this_char) {
			return true;
		}
	}
	return false;
}

bool isIdentifier(char sym) {
	for (char this_char : "_$") {
		if (sym == this_char) {
			return true;
		}
	}
	return std::isalpha(sym);
}

std::optional<shader_precompiler::lexer::Token> shader_precompiler::lexer::LexerStream::next() {

	char nextChar;

	// skip white space
	while (!eof()) {
		nextChar = peek();

		if (nextChar == '\n' || !std::isspace(nextChar)) {
			break;
		}
		else {
			get();
		}
	}

	if (eof()) return std::nullopt;
	if (nextChar == '#') return readDirective();
	if (nextChar == '\"') return readString();
	if (nextChar == '.') {
		get();

		if (std::isdigit(peek())) {
			return readNumber(".");
		}

		return readSymbol(".");
	}
	if (nextChar == '\n') {
		get();
		return createToken(Token::Type::NewLine, "\n");
	}
	if (std::isdigit(nextChar)) return readNumber();
	if (isSymbol(nextChar)) return readSymbol();
	if (isOperator(nextChar)) return readOperator();
	if (isIdentifier(nextChar)) return readIdentifier();

	printError(ErrorCodes::UNEXPECTED_TOKEN, std::string{nextChar} + " UNEXPECTED_TOKEN", line, column);
	return createToken((Token::Type) -1 , std::string{ nextChar });
}

shader_precompiler::lexer::Token shader_precompiler::lexer::LexerStream::readNumber(std::string prefix) {
	std::string buffer = prefix;

	while (!eof()) {
		char c = peek();

		if (!std::isdigit(c) && c != '.') {
			break;
		}

		buffer += get();
	}

	return createToken(Token::Type::Number, buffer);
}

shader_precompiler::lexer::Token shader_precompiler::lexer::LexerStream::readIdentifier() {
	std::string buffer{};

	while (!eof()) {
		char c = peek();

		if (!isIdentifier(c) && !std::isdigit(c)) {
			break;
		}

		buffer += get();
	}

	return createToken(Token::Type::Identifier, buffer);
}
shader_precompiler::lexer::Token shader_precompiler::lexer::LexerStream::readDirective() {
	std::string buffer{};

	buffer += get();

	while (!eof()) {
		char c = peek();

		if (!isIdentifier(c)) {
			break;
		}

		buffer += get();
	}

	return createToken(Token::Type::Directive, buffer);
}
shader_precompiler::lexer::Token shader_precompiler::lexer::LexerStream::readString() {
	std::string buffer{};

	buffer += get();

	while (!eof()) {
		char c = peek();

		buffer += get();

		if (c == '\"' || c == '\n') {
			break;
		}
	}

	return createToken(Token::Type::String, buffer);
}

shader_precompiler::lexer::Token shader_precompiler::lexer::LexerStream::readSymbol(std::string prefix) {

	if (prefix.empty()) {
		return createToken(Token::Type::Symbol, std::string{ get() });
	}
	else {
		return createToken(Token::Type::Symbol, prefix);
	}
}

shader_precompiler::lexer::Token shader_precompiler::lexer::LexerStream::readOperator() {

	return createToken(Token::Type::Operator, std::string{ get() });
}