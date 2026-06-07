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


	while (true) {
		const auto lastToken = currentStream().peek();
		if (!lastToken.has_value()) return std::nullopt;

		if (this->needSkipCode()) {
			currentStream().get();
			continue;
		}
		if (lastToken->type == shader_precompiler::lexer::Token::Type::NewLine || endOfCurrentFile()) {
			currentStream().get();
			startLine = true;
			continue;
		}

		if (lastToken->type == shader_precompiler::lexer::Token::Type::Directive) {
			auto nextToken = currentStream().get().value();
			if (startLine) {
				handleDirective(nextToken);
			}
			else {

			}
			startLine = true;
			continue;
		}
		else if (lastToken->type == shader_precompiler::lexer::Token::Type::Comment) {
			currentStream().get();
			startLine = false;
			continue;
		}
		else if (lastToken->type == shader_precompiler::lexer::Token::Type::Identifier) {
			currentStream().get();

			if (defines.find(lastToken->text) != defines.end())
			{
				auto defineSize = defines.at(lastToken->text).size();
				if (defineSize == 0) {
					printError(shader_precompiler::Error::Level::INFO, shader_precompiler::Error::ErrorCodes::DELETING_BY_DEFINE, shader_precompiler::Error::makeStore(lastToken->text), *lastToken);
				}
				else if (defineSize >= 2) {
					insertDefine = InsertDefine{};
					insertDefine->macroName = lastToken->text;
					insertDefine->tokenNum++;
					startLine = false;
					return defines.at(lastToken->text).at(0);
				}
				else {
					startLine = false;
					return defines.at(lastToken->text).at(0);
				}
			}
			else {
				startLine = false;
				return lastToken;
			}
		}
		startLine = false;
		return currentStream().get();
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
				auto nextToken = currentStream().peek();

				while (nextToken &&
					(nextToken->type != shader_precompiler::lexer::Token::Type::NewLine || endOfCurrentFile())) {
					currentStream().get();
					buffer += nextToken->text + " ";
					nextToken = currentStream().peek();
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

	auto macro = currentStream().get();

	if (macro)
	{
		if (macro->type == shader_precompiler::lexer::Token::Type::String)
		{
			if (directiveToken.text == "#include") {

				auto inclPath = macro.value().text;
				inclPath = inclPath.substr(1, inclPath.size() - 2);

				skipToNewLine();
				readFileInclude(std::filesystem::path(inclPath), macro.value().location);
				return;
			}

		} else if (macro->type == shader_precompiler::lexer::Token::Type::Identifier) {

			auto nextWord = (*macro).text;

			if (directiveToken.text == "#define") {

				std::vector<shader_precompiler::lexer::Token> buffer { };
				auto nextToken = currentStream().peek();

				while (nextToken &&
					nextToken->type != shader_precompiler::lexer::Token::Type::NewLine) {
					if (endOfCurrentFile()) {
						break;
					}

					currentStream().get();

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
					if (endOfCurrentFile()) {
						break;
					}
					nextToken = currentStream().peek();
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
				skipToNewLine();
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
				skipToNewLine();
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
				skipToNewLine();
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
				skipToNewLine();
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
				skipToNewLine();
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
	skipToNewLine();
}

void shader_precompiler::precompiler::PrecompilerLexerStream::readFileInclude(std::filesystem::path filepath, shader_precompiler::Location location) {

	auto relativePath = context.currentPath.parent_path() / filepath;
	if (tryReadFile(relativePath)) {
		return;
	}
	else {
		printError(shader_precompiler::Error::Level::INFO, shader_precompiler::Error::ErrorCodes::CANT_OPEN_FILE, shader_precompiler::Error::makeStore(relativePath.string()), location);
	}

	for (auto& inclDir : context.includeDirectories)
	{
		auto path = inclDir / filepath;
		if (std::filesystem::is_regular_file(path)) {
			if (tryReadFile(path)) {
				return;
			}
			else {
				printError(shader_precompiler::Error::Level::INFO, shader_precompiler::Error::ErrorCodes::CANT_OPEN_FILE, shader_precompiler::Error::makeStore(path.string()), location);
			}
		}
	}
	for (auto& file : context.includeFiles)
	{
		if (filepath.filename() == file.filename()) {
			if (tryReadFile(file)) {
				return;
			}
			else {
				printError(shader_precompiler::Error::Level::INFO, shader_precompiler::Error::ErrorCodes::CANT_OPEN_FILE, shader_precompiler::Error::makeStore(file.string()), location);
			}
		}
	}
}
void shader_precompiler::precompiler::PrecompilerLexerStream::skipToNewLine() {
	if (endOfCurrentFile()) {
		return;
	}
	auto nextToken = currentStream().peek();

	while (nextToken &&
		nextToken->type != shader_precompiler::lexer::Token::Type::NewLine) {
		printError(shader_precompiler::Error::Level::INFO, shader_precompiler::Error::ErrorCodes::TOKENS_AFTER_DIRECTIVE_AND_BEFORE_NEW_LINE, shader_precompiler::Error::makeStore(nextToken->toDebugString()), nextToken->location);
		currentStream().get();
		if (endOfCurrentFile()) {
			return;
		}
		nextToken = currentStream().peek();
	}
	currentStream().get();
	if (endOfCurrentFile()) {
		return;
	}
}
bool shader_precompiler::precompiler::PrecompilerLexerStream::tryReadFile(std::filesystem::path filepath) {
	auto res = fact(filepath);
	if (res == NULL) {
		return false;
	}
	includeStack.push_back(std::move(res));
}

struct FileStreamOpenFileStruct : public shader_precompiler::precompiler::BaseOpenFileStruct {
	std::unique_ptr<shader_precompiler::lexer::BaseLexerStream> stream{};
	std::unique_ptr<std::ifstream> fstream{};
	shader_precompiler::lexer::BaseLexerStream& getStream() override { return *stream; }
};

std::unique_ptr<shader_precompiler::precompiler::BaseOpenFileStruct> shader_precompiler::precompiler::fileStreamOpenFile(std::filesystem::path filepath, shader_precompiler::precompiler::IncludeFactory fact) {
	auto str = std::make_unique<FileStreamOpenFileStruct>();
	str->fstream = std::make_unique<std::ifstream>(filepath);
	if (str->fstream->is_open()) {
		str->stream = fact(*str->fstream);
		return std::move(str);
	}
	return NULL;
}