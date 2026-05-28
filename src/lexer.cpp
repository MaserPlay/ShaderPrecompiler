#include "lexer.hpp"

#include <cctype>

bool isOperator(char sym) {
	for (char this_char : "+-*/=%&^|!?<>") {
		if (sym == this_char) {
			return true;
		}
	}
	return false;
}

bool isSymbol(char sym) {
	for (char this_char : ";,(){}\"[]") {
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
		nextChar = peekChar();

		if (nextChar == '\n' || !std::isspace(nextChar)) {
			break;
		}
		else {
			getChar();
		}
	}

	if (eof()) return std::nullopt;
	if (nextChar == '#') return readDirective();
	if (nextChar == '\"') return readString();
	if (nextChar == '.') {
		getChar();

		if (std::isdigit(peekChar())) {
			return readNumber(".");
		}

		return readSymbol(".");
	}
	if (nextChar == '/') {
		getChar();

		nextChar = peekChar();
		if (peekChar() == '/' || peekChar() == '*') {
			return readCommentStartSlash();
		}

		return readOperator("/");
	}
	if (nextChar == '\n') {
		getChar();
		return createToken(Token::Type::NewLine, "\n");
	}
	if (std::isdigit(nextChar)) return readNumber();
	if (isSymbol(nextChar)) return readSymbol();
	if (isOperator(nextChar)) return readOperator();
	if (isIdentifier(nextChar)) return readIdentifier();


	shader_precompiler::lexer::Token error( (shader_precompiler::lexer::Token::Type) - 1, std::string{ getChar() }, line, column );

	printError(shader_precompiler::Error::Level::WARNING, shader_precompiler::Error::ErrorCodes::UNEXPECTED_CHAR, shader_precompiler::Error::makeStore(nextChar), error);

	return error;
}

shader_precompiler::lexer::Token shader_precompiler::lexer::LexerStream::readCommentStartSlash() {
	std::string buffer = "/";
	bool isMultiLine = (peekChar() == '*');
	char c = peekChar();

	if (isMultiLine) {
		while (!eof()) {

			buffer += getChar();

			if (buffer.back() == '*' && c == '/') {
				break;
			}

			c = peekChar();
		}
	}
	else {
		while (!eof()) {

			buffer += getChar();

			if (c == '\n') {
				break;
			}

			c = peekChar();
		}
	}

	return createToken(Token::Type::Comment, buffer);
}
shader_precompiler::lexer::Token shader_precompiler::lexer::LexerStream::readNumber(std::string prefix) {
	std::string buffer = prefix;
	bool wasDot = prefix.find('.') != std::string::npos;

	while (!eof()) {
		char c = peekChar();

		if (c == '.') {
			if (wasDot) {
				break;
			}
			wasDot = true;
		} else if (!std::isdigit(c)) {
			break;
		} 

		buffer += getChar();
	}

	return createToken(Token::Type::Number, buffer);
}

shader_precompiler::lexer::Token shader_precompiler::lexer::LexerStream::readIdentifier() {
	std::string buffer{};

	while (!eof()) {
		char c = peekChar();

		if (!isIdentifier(c) && !std::isdigit(c)) {
			break;
		}

		buffer += getChar();
	}

	return createToken(Token::Type::Identifier, buffer);
}
shader_precompiler::lexer::Token shader_precompiler::lexer::LexerStream::readDirective() {
	std::string buffer{};

	buffer += getChar();

	while (!eof()) {
		char c = peekChar();

		if (!isIdentifier(c)) {
			break;
		}

		buffer += getChar();
	}

	return createToken(Token::Type::Directive, buffer);
}
shader_precompiler::lexer::Token shader_precompiler::lexer::LexerStream::readString() {
	std::string buffer{};

	buffer += getChar();

	while (!eof()) {
		char c = peekChar();

		buffer += getChar();

		if (c == '\"' || c == '\n') {
			break;
		}
	}

	return createToken(Token::Type::String, buffer);
}

shader_precompiler::lexer::Token shader_precompiler::lexer::LexerStream::readSymbol(std::string prefix) {
	if (prefix.empty()) {
		return createToken(Token::Type::Symbol, std::string{ getChar() });
	}
	else {
		return createToken(Token::Type::Symbol, prefix);
	}
}

shader_precompiler::lexer::Token shader_precompiler::lexer::LexerStream::readOperator(std::string prefix) {
	std::string buffer = prefix;

	buffer += getChar();

	while (!eof()) {
		char c = peekChar();

		if (!isOperator(c)) {
			break;
		}

		buffer += getChar();
	}

	return createToken(Token::Type::Operator, buffer);
}