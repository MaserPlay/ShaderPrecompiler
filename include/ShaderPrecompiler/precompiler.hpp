#pragma once

#include <string>
#include <vector>
#include <map>
#include <sstream>

#include "lexer.hpp"

namespace shader_precompiler::precompiler {

	class PrecompilerLexerStream : public shader_precompiler::lexer::BaseLexerStream {
		BaseLexerStream& from;
		IDiagnosticReporter& reporter;

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

		PRINT_ERROR_DEFINE(shader_precompiler::Error::Stage::PREPROCESSOR)

	public:
		explicit PrecompilerLexerStream(BaseLexerStream& from, IDiagnosticReporter& reporter) : from(from), defines({}),
			numNestedIfdef(0), deleteNestedIfDefs(0), reporter(reporter) {}

		inline bool eof() override {
			return from.eof();
		}
	};
};