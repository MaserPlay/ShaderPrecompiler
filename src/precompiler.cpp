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

namespace precompiler {

	enum class ErrorCodes {
		DIRECTIVE_NOT_FOUND,
		WARNING_STATEMENT
	};

	static void printError(ErrorCodes code, std::string text, std::size_t line, std::size_t column) {
		shader_precompiler::setError(shader_precompiler::Error{
			shader_precompiler::Error::Stage::PREPROCESSOR, (std::size_t)code, text, line, column
			});
	}

	static void replaceAllMacros(const std::vector<std::string>& definesVector, std::map<std::string, std::string>& defines, const std::string& line, const std::function<void(std::string)> writeChars) {

		if (defines.empty()) {
			writeChars(line);
		}
		else {

			for (std::size_t li = 0; li < line.size(); li++)
			{
				if (auto defineOpt = string_utils::isThereAny(line, li, &definesVector[0], std::size(definesVector))) {
					auto define = definesVector[*defineOpt];
					auto macro = defines[define];
					auto nextCharIndex = macro.size() + li + 1;
					if (line.size() < nextCharIndex || (std::isalpha(line[nextCharIndex]) || std::ispunct(line[nextCharIndex]))) {
						writeChars(macro);
						li += define.size() - 1;
						continue;
					}
				}
				writeChars(std::string{ line[li] });
			}
		}
	}

	void process(std::istream& in_code, std::ostringstream& output, const std::map<std::string, std::string>& startupDefines) {

		std::map<std::string, std::string> defines = startupDefines;

		std::vector<std::string> definesVector;
		for (auto [name, define] : defines)
		{
			definesVector.push_back(name);
		}

		bool isStart = true, isInPrecompilerDirective;

		std::size_t numNestedIfdef = 0, // How nested are we?
			deleteNestedIfDefs = 0; // how many nested code we need to delete

		std::size_t lineNum = 0, currentDirective;

		for (std::string line; std::getline(in_code, line);) {
			if (line.empty()) {
				continue;
			}
			lineNum++;

			currentDirective = std::size(directives);
			isInPrecompilerDirective = false;
			for (std::size_t columnNum = 0; columnNum < line.size(); columnNum++)
			{
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

							replaceAllMacros(definesVector, defines, line, [&output](std::string str) {
								output << str;
								});
						}
						columnNum = line.size();
						break;
					}

					if (const auto anyStringIndex = string_utils::isThereAny(line, columnNum, directives, std::size(directives))) {
						columnNum += directives[*anyStringIndex].size();
						std::string nextWord = string_utils::findNextWord(line, columnNum).value_or("");

						switch ((Directives)*anyStringIndex) {
						case Directives::DEFINE: // I.e. define
						{
							definesVector.push_back(nextWord);
							if (line.size() >= columnNum + nextWord.size() + 2) {

								std::string newString{};
								replaceAllMacros(definesVector, defines, std::string{ line, columnNum + nextWord.size() + 2 }, [&newString](std::string str) {
									newString += str;
									});

								defines[nextWord] = newString;
								break;
							}
							else {
								defines[nextWord] = "";
								break;
							}
						}
						case Directives::UNDEF: //I.e. undef
						{
							if (defines.find(nextWord) == end(defines))
							{
								// Trying to undef undefined macro
							}
							else
							{
								definesVector.erase(std::find(begin(definesVector), end(definesVector), nextWord));
								defines.erase(nextWord);
							}
							break;
						}
						case Directives::IFDEF: //I.e. ifdef
						{
							numNestedIfdef++;
							if (defines.find(nextWord) == end(defines))
							{
								deleteNestedIfDefs++;
							}
							else
							{
							}
							break;
						}
						case Directives::IFNDEF: //I.e. ifndef
						{
							numNestedIfdef++;
							if (defines.find(nextWord) == end(defines))
							{
							}
							else
							{
								deleteNestedIfDefs++;
							}
							break;
						}
						case Directives::ELSE: //I.e. else
						{
							if (deleteNestedIfDefs == 1) {
								deleteNestedIfDefs--;
							}
							else if (deleteNestedIfDefs == 0) {
								deleteNestedIfDefs++;
							}
							break;
						}
						case Directives::ELIFDEF: // elifdef
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
						case Directives::ELIFNDEF: // elifndef
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
						case Directives::ENDIF: //I.e. endif
						{
							if (deleteNestedIfDefs > 1) {
								deleteNestedIfDefs--;
							}
							if (numNestedIfdef > 1) {
								numNestedIfdef--;
							}
							break;
						}
						case Directives::WARNING: // #warning
						{
							if (deleteNestedIfDefs == 0) {
								if (line.size() >= columnNum + 1) {
									std::string newString{ line, columnNum + 1 };
									printError(ErrorCodes::WARNING_STATEMENT, newString, lineNum + 1, columnNum + 1);
									break;
								}
							}
						}
						}
						columnNum = line.size();
						break;
					}
					else {
						std::string nextWord = string_utils::findNextWord(line, columnNum).value_or("");
						printError(ErrorCodes::DIRECTIVE_NOT_FOUND, nextWord + " directive not found", lineNum + 1, columnNum + 1);
						columnNum = line.size();
						break;
					}
				}
			}
		}
	}
};