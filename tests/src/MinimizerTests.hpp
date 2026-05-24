#include "../common.hpp"

#include "minimazer.hpp"
#include "diagnostic_reporters.hpp"

static auto processMinimizer(std::string base) {
	shader_precompiler::PrintDiagnostic printDia(shader_precompiler::locales::Locales::ENGLISH);
	std::istringstream iss(base);

	shader_precompiler::lexer::LexerStream tokenStream( iss, printDia );

	shader_precompiler::precompiler::PrecompilerLexerStream afterPreprocessor( tokenStream, printDia );

	shader_precompiler::ast::AstParser ast( afterPreprocessor, printDia );

	shader_precompiler::visitors::MinimazerVisitor min(ast, printDia );

	return min.processTree();
}

TEST(MinimizerTests, a) {

	//auto mini = processMinimizer("int ofpdsayuasdoahdsa = 1; void dosihaa8ysdgayd8s(){}");

	//ASSERT_FALSE(true) << mini->toDebugString(0);
}