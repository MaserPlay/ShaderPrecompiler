#include "precompiler.hpp"

#include <iostream>
#include <vector>
#include <map>
#include <string>

#include "utils/string_utils.hpp"
#include "print_error.hpp"

enum class ErrorCodes {
	DIRECTIVE_NOT_FOUND,
	WARNING_STATEMENT,
	UNEXPECTED_DIRECTIVE_NAME,
	REPLACING_DEFINE_TO_NONE
};

static void printError(ErrorCodes code, std::string text, std::size_t line, std::size_t column) {
	shader_precompiler::setError(shader_precompiler::Error{
		shader_precompiler::Error::Stage::PREPROCESSOR, (std::size_t)code, text, line, column
		});
}

std::optional<shader_precompiler::lexer::Token> shader_precompiler::precompiler::PrecompilerLexerStream::next() {

	if (insertDefine) {
		auto defineSize = defines.at(insertDefine->macroName).size();
		auto nextToken = defines.at(insertDefine->macroName).at(insertDefine->tokenNum);

		if (defineSize == (insertDefine->tokenNum + 1)) { // lastToken
			insertDefine = std::nullopt;
			return nextToken;
		}
		else {
			insertDefine->tokenNum++;
			return nextToken;
		}
	}


	std::optional<shader_precompiler::lexer::Token> lastToken {};

	lastToken = from.peek();
	while (true) {

		if (!lastToken || from.eof()) return std::nullopt;

		if (lastToken->type == shader_precompiler::lexer::Token::Type::NewLine) {
			from.get();
			lastToken = from.peek();

			if (lastToken && lastToken->type == shader_precompiler::lexer::Token::Type::Directive) {
				handleDirective(from.get().value());
			}
		}

		lastToken = from.peek();
		if (lastToken && 
			!this->needSkipCode() &&
			lastToken.value().type != shader_precompiler::lexer::Token::Type::NewLine &&
			lastToken.value().type != shader_precompiler::lexer::Token::Type::Directive) {
			from.get();

			if (lastToken->type == shader_precompiler::lexer::Token::Type::Identifier &&
				defines.find(lastToken->text) != defines.end())
			{
				auto defineSize = defines.at(lastToken->text).size();
				if (defineSize == 0) {
					printError(ErrorCodes::REPLACING_DEFINE_TO_NONE, "Warning: deleting " + lastToken->text + " by #define", 
						lastToken->line, lastToken->column);
				}
				else if (defineSize >= 2) {
					insertDefine = InsertDefine{};
					insertDefine->macroName = lastToken->text;
					insertDefine->tokenNum++;
					return defines.at(lastToken->text).at(0);
				}
				else {
					return defines.at(lastToken->text).at(0);
				}
			}
			else {
				return lastToken;
			}
		}
		if (this->needSkipCode()) {
			from.get();
		}
	}
}
void shader_precompiler::precompiler::PrecompilerLexerStream::handleDirective(const shader_precompiler::lexer::Token& directiveToken) {

	if (directiveToken.text == "#else") {
		if (deleteNestedIfDefs == 1) {
			deleteNestedIfDefs--;
		}
		else if (deleteNestedIfDefs == 0) {
			deleteNestedIfDefs++;
		}
		return;
	}
	else if (directiveToken.text == "#endif")
	{
		if (deleteNestedIfDefs > 1) {
			deleteNestedIfDefs--;
		}
		if (numNestedIfdef > 1) {
			numNestedIfdef--;
		}
		return;
	} else if (directiveToken.text == "#warning") {

		std::string buffer{};
		auto nextToken = from.peek();

		while (nextToken &&
			nextToken->type != shader_precompiler::lexer::Token::Type::NewLine) {
			from.get();
			buffer += nextToken->text + " ";
			nextToken = from.peek();
		}

		printError(ErrorCodes::WARNING_STATEMENT, "Warning: " +
			buffer, directiveToken.line, directiveToken.column);
		return;
	}

	auto macro = from.get();

	if (macro)
	{
		if (macro->type == shader_precompiler::lexer::Token::Type::Identifier) {

			auto nextWord = (*macro).text;

			if (directiveToken.text == "#define") {

				std::vector<shader_precompiler::lexer::Token> buffer { };
				auto nextToken = from.peek();

				while (nextToken &&
					nextToken->type != shader_precompiler::lexer::Token::Type::NewLine) {

					from.get();

					if (nextToken->type == shader_precompiler::lexer::Token::Type::Identifier &&
						defines.find(nextToken->text) != defines.end())
					{
						auto defineSize = defines.at(nextToken->text).size();
						if (defineSize == 0) {
							printError(ErrorCodes::REPLACING_DEFINE_TO_NONE, "Warning: deleting " + nextToken->text + " by #define",
								nextToken->line, nextToken->column);
						}
						else {
							for (auto& i : defines.at(nextToken->text)) {
								buffer.push_back(i);
							}
						}
					}
					else {
						buffer.push_back(*nextToken);
					}
					nextToken = from.peek();
				}

				defines[macro->text] = buffer;
				return;
			}
			else if (directiveToken.text == "#undef") {
				if (defines.find(nextWord) == end(defines))
				{
					// Trying to undef undefined macro
				}
				else
				{
					defines.erase(nextWord);
				}
				return;
			}
			else if (directiveToken.text == "#ifdef") {
				if (defines.find(nextWord) == end(defines))
				{
					deleteNestedIfDefs++;
				}
				else
				{

				}
				return;
			}
			else if (directiveToken.text == "#ifndef") {
				if (defines.find(nextWord) == end(defines))
				{
				}
				else
				{
					deleteNestedIfDefs++;
				}
				return;
			}
			else if (directiveToken.text == "#elifdef") {
				if (deleteNestedIfDefs == 1) {

					if (defines.find(nextWord) == end(defines))
					{
						deleteNestedIfDefs--;
					}
					else
					{
					}
				}
				else if (deleteNestedIfDefs == 0) {
					if (defines.find(nextWord) == end(defines))
					{
						deleteNestedIfDefs++;
					}
					else
					{
					}
				}
				return;
			}
			else if (directiveToken.text == "#elifndef") {
				if (deleteNestedIfDefs == 1) {

					if (defines.find(nextWord) == end(defines))
					{
					}
					else
					{
						deleteNestedIfDefs--;
					}
				}
				else if (deleteNestedIfDefs == 0) {
					if (defines.find(nextWord) == end(defines))
					{
					}
					else
					{
						deleteNestedIfDefs++;
					}
				}
				return;
			}
		}
		else {
			printError(ErrorCodes::UNEXPECTED_DIRECTIVE_NAME, "Unexpected macro name: unexpected token " +
				macro->text, macro->line, macro->column);
		}
	}
	else {
		printError(ErrorCodes::UNEXPECTED_DIRECTIVE_NAME, "Unexpected macro name: No next token.", directiveToken.line, directiveToken.column);
	}

	printError(ErrorCodes::UNEXPECTED_DIRECTIVE_NAME, "Unexpected directive name " + directiveToken.text, directiveToken.line, directiveToken.column);
}