#include "../common.hpp"

#include "minimazer.hpp"
#include "semantic.hpp"
#include "to_glsl.hpp"

static auto processGlsl(std::string base) {
	std::istringstream iss(base);
	shader_precompiler::PrintDiagnostic da(shader_precompiler::locales::Locales::ENGLISH);

	shader_precompiler::lexer::LexerStream tokenStream(iss, da);

	shader_precompiler::precompiler::PrecompilerLexerStream afterPreprocessor(tokenStream, da);

	shader_precompiler::ast::AstParser ast(afterPreprocessor, da);

	shader_precompiler::visitors::MinimazerVisitor min(ast, da);

	shader_precompiler::SemanticVisitor sem(min, da);

	std::ostringstream ss{};
	shader_precompiler::GlslVisitor glsl(sem, da, ss);

	glsl.generate();

	return ss.str();
}

TEST(GlslTests, minimal) {
	auto str = processGlsl("int a = 1;void main(){}");
	ASSERT_EQ(str, "#version 330 core\nint q=1;void main(){}");
}

TEST(GlslTests, allAttributes) {
	auto str = processGlsl("[[__glsl_in]] int a;[[__glsl_out]] int b;[[__glsl_uniform]] int c;[[__glsl_layout_in(0)]] int ausyadgt8asd87ads;void main(){}");
	ASSERT_EQ(str, "#version 330 core\nin int q;out int w;uniform int e;layout(location=0)in int r;void main(){}");
}