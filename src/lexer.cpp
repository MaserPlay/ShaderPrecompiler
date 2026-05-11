#include "lexer.hpp"

#include <cctype>

constexpr auto eof = std::istream::traits_type::eof();

bool isOperator(char sym) {
	for (char this_char : "+-*/=") {
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

bool isName(char sym) {
	return std::isalpha(sym) ||
		sym == '_';
}

std::optional<shader_precompiler::lexer::Token> shader_precompiler::lexer::LexerStream::next() {

	shader_precompiler::lexer::Token::Type type = (shader_precompiler::lexer::Token::Type)-1;

	buffer.clear();
	while (!input.eof()) {
		column++;

		if (nextChar == -1) {

		}
		else if (nextChar == '#') {
			if (buffer.empty()) {
				type = shader_precompiler::lexer::Token::Type::Directive;
			}
			buffer += nextChar;
		}
		else if (nextChar == '.') {
			if (buffer.empty()) {
				type = shader_precompiler::lexer::Token::Type::Symbol;
				buffer += nextChar;
			}
			else if (type == shader_precompiler::lexer::Token::Type::Number) {
				buffer += nextChar;
			}
			else {
				break;
			}
		}
		else if (nextChar == '\n') {
			if (buffer.empty()) {

				line++;
				column = 0;

				type = shader_precompiler::lexer::Token::Type::NewLine;
				buffer += nextChar;
				nextChar = input.get();
			}
			else {
				column--;
			}
			break;
		}
		else if (isSymbol(nextChar)) {
			if (buffer.empty()) {
				type = shader_precompiler::lexer::Token::Type::Symbol;
				buffer += nextChar;
				nextChar = input.get();
			}
			else {
				column--;
			}
			break;
		}
		else if (isOperator(nextChar)) {
			if (buffer.empty()) {
				type = shader_precompiler::lexer::Token::Type::Operator;
				buffer += nextChar;
				nextChar = input.get();
			}
			break;
		}
		else if (std::isspace(nextChar)) {
			if (!buffer.empty()) {
				nextChar = input.get();
				break;
			}
		}
		else if (isName(nextChar)) {
			if (buffer.empty()) {
				type = shader_precompiler::lexer::Token::Type::Identifier;
				buffer += nextChar;
			}
			else if (type == shader_precompiler::lexer::Token::Type::Identifier ||
					type == shader_precompiler::lexer::Token::Type::Directive) {
				buffer += nextChar;
			}
			else {
				break;
			}
		}
		else if (std::isdigit(nextChar)) {
			if (
				(type == shader_precompiler::lexer::Token::Type::Symbol && buffer.size() == 1 && buffer[0] == '.') || 
				buffer.empty()) {
				type = shader_precompiler::lexer::Token::Type::Number;
			}
			buffer += nextChar;
		}
		nextChar = input.get();
	}

	if (buffer.empty()) {
		return std::nullopt;
	}

	return Token(type, buffer, line, column);
}