#pragma once

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <memory>
#include <filesystem>
#include <fstream>

#include "lexer.hpp"

namespace shader_precompiler::precompiler {
	struct BaseOpenFileStruct {
		virtual shader_precompiler::lexer::BaseLexerStream& getStream() = 0;
	};
	using OpenFileFactory = std::function<std::unique_ptr<BaseOpenFileStruct>(std::filesystem::path)>;

	struct Context {
		std::vector<std::filesystem::path> includeDirectories{};
		std::vector<std::filesystem::path> includeFiles{};
		std::filesystem::path currentPath{};
	};

	class PrecompilerLexerStream : public shader_precompiler::lexer::BaseLexerStream {
		BaseLexerStream& from;
		std::vector<std::unique_ptr<BaseOpenFileStruct>> includeStack{};
		IDiagnosticReporter& reporter;
		OpenFileFactory fact;
		const Context context;
		bool startLine;

		std::optional<shader_precompiler::lexer::Token> next() override;
		std::map<std::string, std::vector<shader_precompiler::lexer::Token>> defines;
		std::size_t numNestedIfdef, // How nested are we?
			deleteNestedIfDefs; // How many nested code we need to delete?

		struct InsertDefine {
			std::string macroName;
			std::size_t tokenNum {0};
		};
		std::optional<InsertDefine> insertDefine;

		void handleDirective(const shader_precompiler::lexer::Token& tok);
		inline bool needSkipCode() { return deleteNestedIfDefs != 0; }

		bool endOfCurrentFile() {
			while (!includeStack.empty()) {
				auto& next = includeStack.back()->getStream();
				if (!next.peek().has_value()) {
					return true;
				}
				else {
					return false;
				}
			}

			return false;
		}

		inline BaseLexerStream& currentStream() {
			while (!includeStack.empty()) {
				auto& next = includeStack.back()->getStream();
				if (!next.peek().has_value()) {
					includeStack.pop_back();
				}
				else {
					return includeStack.back()->getStream();
				}
			}

			return from;
		}

		void skipToNewLine();

		void readFileInclude(std::filesystem::path filepath, shader_precompiler::Location location);
		// return - isSucces?
		bool tryReadFile(std::filesystem::path filepath);

		PRINT_ERROR_DEFINE(shader_precompiler::Error::Stage::PREPROCESSOR)
	public:
		explicit PrecompilerLexerStream(BaseLexerStream& from, IDiagnosticReporter& reporter, OpenFileFactory fact, const Context context) : from(from), defines({}),
			numNestedIfdef(0), deleteNestedIfDefs(0), reporter(reporter), fact(fact), context(context), startLine(true){}

		inline bool eof() override {
			return currentStream().eof();
		}
	};

	using IncludeFactory =
		std::function<std::unique_ptr<shader_precompiler::lexer::BaseLexerStream>(std::istream&)>;
	std::unique_ptr<shader_precompiler::precompiler::BaseOpenFileStruct> fileStreamOpenFile(std::filesystem::path filepath, shader_precompiler::precompiler::IncludeFactory fact);
};