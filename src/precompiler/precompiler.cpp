#include "precompiler/precompiler.hpp"

#include <iostream>
#include <vector>
#include <map>
#include <string>

#include "utils/string_utils.hpp"
#include "print_error.hpp"

const std::string directives[]{ "define", "undef", "ifdef", "ifndef", "else", "elifdef", "elifndef", "endif" };

namespace precompiler {

	enum class ErrorCodes {
		DIRECTIVE_NOT_FOUND
	};

	static void printError(ErrorCodes code, std::string text, std::size_t line, std::size_t column) {
		shader_precompiler::setError(shader_precompiler::Error{ 
			shader_precompiler::Error::Stage::PREPROCESSOR, (std::size_t) code, text, line, column
			});
	}

	std::string process(std::istream& in_code, const std::map<std::string, std::string>& startupDefines) {
		std::string outCode = "";

		std::map<std::string, std::string> defines = startupDefines;
		bool deleteLines = false;

		for (std::string line; std::getline(in_code, line);) {
			if (line.empty()) {
				continue;
			}

			std::size_t lineNum = 0, currentDirective = std::size(directives);
			bool isPrecompilerDirective = false;
			for (std::size_t column = 0; column < line.size(); column++)
			{
				lineNum++;
				if (!std::isspace(line[column])) {
					if (line[column] == '#') {
						isPrecompilerDirective = true;
						continue;
					}
					else if (!isPrecompilerDirective) {
						if (!deleteLines) {
							if (!outCode.empty()) {
								outCode += '\n';
							}
							outCode += line;
						}
						break;
					}
					else if (currentDirective != std::size(directives)) {

						std::string word{};
						for (std::size_t wi = column; wi < line.size(); wi++)
						{
							if (!std::isspace(line[wi])) {
								word += line[wi];
							}
							else {
								break;
							}
						}

						switch (currentDirective) {
							case 0: // I.e. define
							{
								defines[word] = "";
								break;
							}
							case 1: //I.e. undef
							{
								if (defines.find(word) == end(defines))
								{
									// Trying to undef undefined macro
								}
								else
								{
									defines.erase(word);
								}
								break;
							}
							case 2: //I.e. ifdef
							{
								//defines.find
								if (defines.find(word) == end(defines))
								{
									deleteLines = true;
								}
								else
								{
								}
								break;
							}
							case 3: //I.e. ifndef
							{
								//defines.find
								if (defines.find(word) == end(defines))
								{
								}
								else
								{
									deleteLines = true;
								}
								break;
							}
						}
						break;
					}
					else {
						for (std::size_t d = 0; d < std::size(directives); d++)
						{
							std::string directive = directives[d];
							bool isWord = false;
							if (line.size() >= column + directive.size()) {
								for (std::size_t ii = 0; ii < directive.size(); ii++)
								{
									if (directive[ii] == line[ii + column]) {
										isWord = true;
									}
									else {
										isWord = false;
										break;
									}
								}
							}

							if (isWord) {
								currentDirective = d;
								column += directive.size();
								break;
							}
						}
						if (currentDirective != std::size(directives)) {
							switch (currentDirective) {
								case 4: //I.e. else
								{
									deleteLines = !deleteLines;
									break;
								}
								case 7: //I.e. endif
								{
									deleteLines = false;
									break;
								}
							}
						}
						else {
							// PrintError
							std::string word{};
							bool inWord = false;
							for (std::size_t wi = column; wi < line.size(); wi++)
							{
								if (!std::isspace(line[wi])) {
									if (!inWord) {
										inWord = true;
									}
									word += line[wi];
								}
								else {
									if (inWord) {
										break;
									}
								}
							}
							printError(ErrorCodes::DIRECTIVE_NOT_FOUND, word + " directive not found", lineNum + 1, column + 1);
							break;
						}
					}
				}
			}
		}

		return outCode;
	}
};