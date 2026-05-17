#include "../common.hpp"

#include "precompiler.hpp"
#include "ast.hpp"

static auto processAst(std::string base) {
	std::istringstream iss(base);

	shader_precompiler::lexer::LexerStream tokenStream{ iss };

	shader_precompiler::precompiler::PrecompilerLexerStream afterPreprocessor{ tokenStream };

	shader_precompiler::ast::AstParser ast{ afterPreprocessor };

	return ast.createTree();
}

TEST(AstTests, SimplyExpression) {
	auto tree = processAst("int a = 2+2;");

	auto rightTree = new shader_precompiler::ast::nodes::CodeBlock();
	rightTree->expressions.push_back(
		std::make_unique<shader_precompiler::ast::nodes::Operator>(
			std::make_unique< shader_precompiler::ast::nodes::VariableInitialization>(
				std::make_unique< shader_precompiler::ast::nodes::Identifier>("int"),
				std::make_unique< shader_precompiler::ast::nodes::Identifier>("a")
			),
			"=",
			std::make_unique<shader_precompiler::ast::nodes::Operator>(
				std::make_unique< shader_precompiler::ast::nodes::NumberExpr>(2),
				"+",
				std::make_unique< shader_precompiler::ast::nodes::NumberExpr>(2)
			)
		)
	);

	ASSERT_TRUE(rightTree->equals(*tree)) << tree->toDebugString(0);
}

TEST(AstTests, RightOperationOrderExpression) {
	auto tree = processAst("int a = 2+(2*2);");

	auto rightTree = new shader_precompiler::ast::nodes::CodeBlock();
	rightTree->expressions.push_back(
		std::make_unique<shader_precompiler::ast::nodes::Operator>(
			std::make_unique< shader_precompiler::ast::nodes::VariableInitialization>(
				std::make_unique< shader_precompiler::ast::nodes::Identifier>("int"),
				std::make_unique< shader_precompiler::ast::nodes::Identifier>("a")
			),
			"=",
			std::make_unique<shader_precompiler::ast::nodes::Operator>(
					std::make_unique<shader_precompiler::ast::nodes::Operator>(
						std::make_unique< shader_precompiler::ast::nodes::NumberExpr>(2),
						"+",
						std::make_unique< shader_precompiler::ast::nodes::NumberExpr>(2)
					),
				"*",
				std::make_unique< shader_precompiler::ast::nodes::NumberExpr>(2)
			)
		)
	);

	ASSERT_TRUE(rightTree->equals(*tree)) << "Actual: " + tree->toDebugString(0) + 
		"\nExpected: " + rightTree->toDebugString(0);
}

TEST(AstTests, EmptyFunc) {
	auto tree = processAst("void main(){}");


	auto rightTree = new shader_precompiler::ast::nodes::CodeBlock();
	rightTree->expressions.push_back(
		std::make_unique<shader_precompiler::ast::nodes::Func>(
			std::make_unique< shader_precompiler::ast::nodes::Identifier>("void"),
			std::make_unique< shader_precompiler::ast::nodes::Identifier>("main"),
			std::make_unique< shader_precompiler::ast::nodes::CodeBlock>(),
			false
		)
	);

	ASSERT_TRUE(rightTree->equals(*tree)) << tree->toDebugString(0);
}