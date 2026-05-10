#include "precompiler/precompiler.hpp"

#include <iostream>
#include <vector>
#include <map>
#include <string>

#include "utils/string_utils.hpp"
#include "print_error.hpp"

const std::string directives[]{ "define", "undef", "ifdef", "ifndef", "else", "elifdef", "elifndef", "endif", "warning"};

namespace precompiler {

	enum class ErrorCodes {
		DIRECTIVE_NOT_FOUND,
		ERROR_STATEMENT
	};

	static void printError(ErrorCodes code, std::string text, std::size_t line, std::size_t column) {
		shader_precompiler::setError(shader_precompiler::Error{
			shader_precompiler::Error::Stage::PREPROCESSOR, (std::size_t) code, text, line, column
			});
	}

	void process(std::istream& in_code, std::ostringstream& output, const std::map<std::string, std::string>& startupDefines) {

		std::map<std::string, std::string> defines = startupDefines;
		bool isStart = true, isInPrecompilerDirective;

		std::size_t numNestedIfdef = 0, // How nested are we?
			deleteNestedIfDefs = 0; // how many nested code we need to delete
		
		std::size_t lineNum, currentDirective;

		for (std::string line; std::getline(in_code, line);) {
			if (line.empty()) {
				continue;
			}

			lineNum = 0, currentDirective = std::size(directives);
			isInPrecompilerDirective = false;
			for (std::size_t columnNum = 0; columnNum < line.size(); columnNum++)
			{
				lineNum++;
				if (!std::isspace(line[columnNum])) {
					if (line[columnNum] == '#') {
						isInPrecompilerDirective = true;
						continue;
					}
					else if (!isInPrecompilerDirective) {
						if (deleteNestedIfDefs == 0) {
							if (!isStart) {
								output << '\n';
							}
							isStart = false;
							output << line;
						}
						break;
					}
					else {
						const auto anyStringIndex = string_utils::isThereAny(line, columnNum, directives, std::size(directives));
						if (anyStringIndex.has_value()) {
							columnNum += directives[*anyStringIndex].size();
							std::string nextWord = string_utils::findNextWord(line, columnNum, true).value_or("");

							switch (*anyStringIndex) {
							case 0: // I.e. define
							{
								defines[nextWord] = "";
								break;
							}
							case 1: //I.e. undef
							{
								if (defines.find(nextWord) == end(defines))
								{
									// Trying to undef undefined macro
								}
								else
								{
									defines.erase(nextWord);
								}
								break;
							}
							case 2: //I.e. ifdef
							{
								numNestedIfdef++;
								//defines.find
								if (defines.find(nextWord) == end(defines))
								{
									deleteNestedIfDefs++;
								}
								else
								{
								}
								break;
							}
							case 3: //I.e. ifndef
							{
								numNestedIfdef++;
								//defines.find
								if (defines.find(nextWord) == end(defines))
								{
								}
								else
								{
									deleteNestedIfDefs++;
								}
								break;
							}
							case 4: //I.e. else
							{
								if (deleteNestedIfDefs == 1) {
									deleteNestedIfDefs--;
								}
								else if (deleteNestedIfDefs == 0) {
									deleteNestedIfDefs++;
								}
								break;
							}
							case 5: // elifdef
							{
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
								break;
							}
							case 6: // elifndef
							{
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
								break;
							}
							case 7: //I.e. endif
							{
								deleteNestedIfDefs--;
								numNestedIfdef--;
								break;
							}
							case 8: // #warning
							{
								printError(ErrorCodes::ERROR_STATEMENT, nextWord, lineNum, columnNum + 1);
							}
							}
							break;
						}
						else {
							std::string nextWord = string_utils::findNextWord(line, columnNum, true).value_or("");
							// PrintError
							printError(ErrorCodes::DIRECTIVE_NOT_FOUND, nextWord + " directive not found", lineNum, columnNum + 1);
							break;
						}
					}
				}
			}
		}
	}
};