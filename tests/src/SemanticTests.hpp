#include "../common.hpp"
#include "diagnostic_reporters.hpp"
#include "semantic.hpp"

#include <string>
#include <functional>

static auto processSemantic(std::string base, SemanticDiagnostic& da, std::function<void(shader_precompiler::SemanticVisitor&)> actionWithSemantic = [](shader_precompiler::SemanticVisitor&){}) {
	shader_precompiler::PrintDiagnostic pr(shader_precompiler::locales::Locales::ENGLISH);
	std::istringstream iss(base);

	shader_precompiler::lexer::LexerStream tokenStream(iss, pr);

	shader_precompiler::precompiler::PrecompilerLexerStream afterPreprocessor(tokenStream, pr);

	shader_precompiler::ast::AstParser ast(afterPreprocessor, pr);

	shader_precompiler::SemanticVisitor sem(ast, da);
	actionWithSemantic(sem);

	return sem.processTree();
}

TEST(SemanticTests, NoErrors) {
	SemanticDiagnostic da{};
	auto tree = processSemantic("int a = 1; void idposahjoadshaodis(){a = 1;}", da);

	ASSERT_LE(da.getErrorsCount(shader_precompiler::Error::Level::FATAL), 0) << tree->toDebugString(0);
}

TEST(SemanticTests, Predeclareted) {
	SemanticDiagnostic da{};
	auto tree = processSemantic("vec3 v; void main(){vec3 two = vec3(1,1,1);}", da, [](shader_precompiler::SemanticVisitor& sem) {
		sem.addTypes("vec3");
		sem.addFunctions(shader_precompiler::SemanticVisitor::Func{ "vec3", "vec3", {
			shader_precompiler::SemanticVisitor::Variable{"float", "x"},
			shader_precompiler::SemanticVisitor::Variable{"float", "y"},
			shader_precompiler::SemanticVisitor::Variable{"float", "z"}
			} });
		});

	ASSERT_LE(da.getErrorsCount(shader_precompiler::Error::Level::FATAL), 0) << tree->toDebugString(0);
}

TEST(SemanticTests, MultipleErrors) {
	SemanticDiagnostic da{};
	auto tree = processSemantic("re6rte6e6 a = 1; pdashdhoiudash idposahjoadshaodis(){ldsnauid = klab;kgfihasdf();}", da);

	ASSERT_GE(da.getErrorsCount(shader_precompiler::Error::Level::FATAL), 5) << tree->toDebugString(0);
}