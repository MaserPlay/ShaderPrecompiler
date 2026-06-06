#include "precompiler.hpp"

#include <iostream>
#include <vector>
#include <map>
#include <string>

#include "utils/string_utils.hpp"

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

		if (!lastToken.has_value()) return std::nullopt;

		if (lastToken->type == shader_precompiler::lexer::Token::Type::NewLine) {
			from.get();
			lastToken = from.peek();

			if (lastToken && lastToken->type == shader_precompiler::lexer::Token::Type::Directive) {
				handleDirective(from.get().value());
			}
		} else if (lastToken->type == shader_precompiler::lexer::Token::Type::Comment) {
			from.get();
		}
		else if (lastToken->type == shader_precompiler::lexer::Token::Type::Directive) {
			printError(shader_precompiler::Error::Level::WARNING, shader_precompiler::Error::ErrorCodes::UNEXPECTED_TOKEN, shader_precompiler::Error::makeStore(lastToken->text), *lastToken);
			from.get();
		}

		lastToken = from.peek();
		if (lastToken && 
			!this->needSkipCode() &&
			lastToken.value().type != shader_precompiler::lexer::Token::Type::NewLine &&
			lastToken.value().type != shader_precompiler::lexer::Token::Type::Directive &&
			lastToken.value().type != shader_precompiler::lexer::Token::Type::Comment) {
			from.get();

			if (lastToken->type == shader_precompiler::lexer::Token::Type::Identifier &&
				defines.find(lastToken->text) != defines.end())
			{
				auto defineSize = defines.at(lastToken->text).size();
				if (defineSize == 0) {
					printError(shader_precompiler::Error::Level::INFO, shader_precompiler::Error::ErrorCodes::DELETING_BY_DEFINE, shader_precompiler::Error::makeStore(lastToken->text), *lastToken);
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
	}
	else {
		for (auto& str : { "#warning", "#info", "#error", "#fatal" })
		{
			if (directiveToken.text == str) {
				std::string buffer{};
				auto nextToken = from.peek();

				while (nextToken &&
					nextToken->type != shader_precompiler::lexer::Token::Type::NewLine) {
					from.get();
					buffer += nextToken->text + " ";
					nextToken = from.peek();
				}

				shader_precompiler::Error::Level level;
				switch (str[1]) {
				case 'w': level = shader_precompiler::Error::Level::WARNING; break;
				case 'i': level = shader_precompiler::Error::Level::INFO; break;
				case 'e': level = shader_precompiler::Error::Level::ERROR; break;
				case 'f': level = shader_precompiler::Error::Level::FATAL; break;
				}

				printError(level, shader_precompiler::Error::ErrorCodes::LEVEL_DIRECTIVE, shader_precompiler::Error::makeStore(buffer), *nextToken);
				return;
			}
		}
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
							printError(shader_precompiler::Error::Level::INFO, shader_precompiler::Error::ErrorCodes::DELETING_BY_DEFINE, shader_precompiler::Error::makeStore(nextToken->text), *nextToken);
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
			printError(shader_precompiler::Error::Level::ERROR, shader_precompiler::Error::ErrorCodes::UNEXPECTED_MACRO_NAME, shader_precompiler::Error::makeStore(macro->text), *macro);
		}
	}
	else {
		printError(shader_precompiler::Error::Level::ERROR, shader_precompiler::Error::ErrorCodes::NO_DIRECTIVE_NAME, shader_precompiler::Error::makeStore(), directiveToken);
	}

	printError(shader_precompiler::Error::Level::ERROR, shader_precompiler::Error::ErrorCodes::UNEXPECTED_DIRECTIVE, shader_precompiler::Error::makeStore(directiveToken.text), directiveToken);
}