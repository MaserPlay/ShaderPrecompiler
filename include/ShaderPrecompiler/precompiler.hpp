#pragma once

#include <string>
#include <vector>
#include <map>
#include <sstream>

#include "lexer.hpp"

namespace shader_precompiler::precompiler {

	class PrecompilerLexerStream : public shader_precompiler::lexer::BaseLexerStream {
		BaseLexerStream& from;
	public:
		explicit PrecompilerLexerStream(BaseLexerStream& from) : from(from) {}
		std::optional<shader_precompiler::lexer::Token> next() override;
	};
};