#include "../common.hpp"
#include "diagnostic_reporters.hpp"
#include "semantic.hpp"

#include <string>
#include <functional>

static auto processSemantic(std::string base, shader_precompiler::CalcDiagnostic& da, std::function<void(shader_precompiler::SemanticVisitor&)> actionWithSemantic = [](shader_precompiler::SemanticVisitor&){}) {
	shader_precompiler::PrintDiagnostic pr(shader_precompiler::locales::Locales::ENGLISH);
	std::istringstream iss(base);

	shader_precompiler::lexer::LexerStream tokenStream(iss, pr);

	shader_precompiler::precompiler::PrecompilerLexerStream afterPreprocessor(tokenStream, pr, emptyFileFactory, shader_precompiler::precompiler::Context{});

	shader_precompiler::ast::AstParser ast(afterPreprocessor, pr);

	shader_precompiler::SemanticVisitor sem(ast, da);
	sem.addShaderTypesFunctions();
	actionWithSemantic(sem);

	return sem.processTree();
}

TEST(SemanticTests, NoErrors) {
	shader_precompiler::PrintDiagnostic pr(shader_precompiler::locales::Locales::ENGLISH);
	shader_precompiler::CalcDiagnostic da(pr);
	auto tree = processSemantic("int a = 1; void idposahjoadshaodis(){a = 1;}", da);

	ASSERT_LE(da.getErrorsCount(shader_precompiler::Error::Level::FATAL), 0) << shader_precompiler::ast::toDebugString(tree);
}

TEST(SemanticTests, Predeclareted) {
	shader_precompiler::PrintDiagnostic pr(shader_precompiler::locales::Locales::ENGLISH);
	shader_precompiler::CalcDiagnostic da(pr);
	auto tree = processSemantic("vec3 v; void main(){vec3 two = vec3(1,1,1);}", da, [](shader_precompiler::SemanticVisitor& sem) {
		sem.addTypes(shader_precompiler::SemanticVisitor::Type("vec3"));
		sem.addFunctions(shader_precompiler::SemanticVisitor::Func{ "vec3", "vec3", {
			shader_precompiler::SemanticVisitor::Variable{"float", "x"},
			shader_precompiler::SemanticVisitor::Variable{"float", "y"},
			shader_precompiler::SemanticVisitor::Variable{"float", "z"}
			} });
		});

	ASSERT_LE(da.getErrorsCount(shader_precompiler::Error::Level::FATAL), 0) << shader_precompiler::ast::toDebugString(tree);
}

TEST(SemanticTests, MultipleErrors) {
	shader_precompiler::PrintDiagnostic pr(shader_precompiler::locales::Locales::ENGLISH);
	shader_precompiler::CalcDiagnostic da(pr);
	auto tree = processSemantic("re6rte6e6 a = 1; pdashdhoiudash idposahjoadshaodis(){ldsnauid = klab;kgfihasdf();} void idposahjoadshaodis(){}", da);

	ASSERT_GE(da.getErrorsCount(shader_precompiler::Error::Level::FATAL), 6) << shader_precompiler::ast::toDebugString(tree);
}

TEST(SemanticTests, ImposibleCode) {
	shader_precompiler::PrintDiagnostic pr(shader_precompiler::locales::Locales::ENGLISH);
	shader_precompiler::CalcDiagnostic da(pr);
	auto tree = processSemantic("s;a;a;a;;a;a;;a;a;a;a;;;;;;a;;a;a;;a;a;a;;a;a;as[odj] d]a]d ad[d[ads] a[sd ][a]ds [as]d [a]sd[ a]sd[[ [[ []] ]][ [[ [] ][] [] [ ][ ][ ] [[] [][ ] [] [][ ]i  shda9uhd sahs9d 9iauhsd 23y r78 ty 8df6yy7fassdaff7a 76sd67asdf ; 'qk4t[ik3t4ep[ju[a'ufa[f'afdh;adfhlFH;ASDFHf'EAH;", da);

	ASSERT_GE(da.getErrorsCount(shader_precompiler::Error::Level::FATAL), 0) << shader_precompiler::ast::toDebugString(tree);
}