#include "precompiler.hpp"

#include <iostream>
#include <vector>
#include <map>
#include <string>

#include "utils/string_utils.hpp"
#include "print_error.hpp"

const std::string directives[]{ "define", "undef", "ifdef", "ifndef", "else", "elifdef", "elifndef", "endif", "warning" };

enum class Directives
{
	DEFINE,
	UNDEF,
	IFDEF,
	IFNDEF,
	ELSE,
	ELIFDEF,
	ELIFNDEF,
	ENDIF,
	WARNING
};

enum class ErrorCodes {
	DIRECTIVE_NOT_FOUND,
	WARNING_STATEMENT
};

static void printError(ErrorCodes code, std::string text, std::size_t line, std::size_t column) {
	shader_precompiler::setError(shader_precompiler::Error{
		shader_precompiler::Error::Stage::PREPROCESSOR, (std::size_t)code, text, line, column
		});
}

std::optional<shader_precompiler::lexer::Token> shader_precompiler::precompiler::PrecompilerLexerStream::next() {
	return from.next();
}