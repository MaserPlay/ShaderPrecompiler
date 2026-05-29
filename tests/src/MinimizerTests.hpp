#include "../common.hpp"

#include "minimazer.hpp"
#include "semantic.hpp"

static auto processMinimizer(std::string base, SemanticDiagnostic& da, std::function<void(shader_precompiler::SemanticVisitor&)> actionWithSemantic = [](shader_precompiler::SemanticVisitor&) {}) {
	std::istringstream iss(base);

	shader_precompiler::lexer::LexerStream tokenStream( iss, da);

	shader_precompiler::precompiler::PrecompilerLexerStream afterPreprocessor( tokenStream, da);

	shader_precompiler::ast::AstParser ast( afterPreprocessor, da);

	shader_precompiler::visitors::MinimazerVisitor min(ast, da);

	shader_precompiler::SemanticVisitor sem(min, da);
	actionWithSemantic(sem);

	return sem.processTree();
}

TEST(MinimizerTests, MinimalCode) {
	SemanticDiagnostic da{};
	auto tree = processMinimizer("int ofpdsayuasdoahdsa = 1; void dosihaa8ysdgayd8s(){ofpdsayuasdoahdsa = 10100101;} void main(){dosihaa8ysdgayd8s()}", da);

	ASSERT_LE(da.getErrorsCount(shader_precompiler::Error::Level::FATAL), 0) << shader_precompiler::ast::toDebugString(tree);
}

TEST(MinimizerTests, Skip) {
	SemanticDiagnostic da{};
	auto tree = processMinimizer("[[__minimazer_skip]] int ofpdsayuasdoahdsa = 1; [[__minimazer_skip]] void dosihaa8ysdgayd8s(){ofpdsayuasdoahdsa = 10100101;} void main(){dosihaa8ysdgayd8s(); ofpdsayuasdoahdsa = 38658243;}", da);

	ASSERT_LE(da.getErrorsCount(shader_precompiler::Error::Level::FATAL), 0) << shader_precompiler::ast::toDebugString(tree);
}