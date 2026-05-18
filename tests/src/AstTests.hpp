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
template<class T, class... Args>
std::vector<std::unique_ptr<T>> makeVector(Args&&... args) {
	std::vector<std::unique_ptr<T>> vec;
	(vec.push_back(std::forward<Args>(args)), ...);
	return vec;
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

	*rightTree == *tree;
	ASSERT_EQ(*rightTree, *tree) << tree->toDebugString(0);
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
				std::make_unique< shader_precompiler::ast::nodes::NumberExpr>(2),
				"+",
				std::make_unique<shader_precompiler::ast::nodes::Operator>(
					std::make_unique< shader_precompiler::ast::nodes::NumberExpr>(2),
					"*",
					std::make_unique< shader_precompiler::ast::nodes::NumberExpr>(2)
				)
			)
		)
	);

	ASSERT_EQ(*rightTree, *tree) << "Actual: " + tree->toDebugString(0) +
		"\nExpected: " + rightTree->toDebugString(0);
}

TEST(AstTests, EmptyFunc) {
	auto tree = processAst("void main();void main(){}");


	auto rightTree = new shader_precompiler::ast::nodes::CodeBlock();
	rightTree->expressions.push_back(
		std::make_unique<shader_precompiler::ast::nodes::FuncDeclaration>(
			std::make_unique< shader_precompiler::ast::nodes::Identifier>("void"),
			std::make_unique< shader_precompiler::ast::nodes::Identifier>("main"),
			makeVector< shader_precompiler::ast::nodes::VariableInitialization>()
		)
	);
	rightTree->expressions.push_back(
		std::make_unique<shader_precompiler::ast::nodes::Func>(
			std::make_unique<shader_precompiler::ast::nodes::FuncDeclaration>(
				std::make_unique< shader_precompiler::ast::nodes::Identifier>("void"),
				std::make_unique< shader_precompiler::ast::nodes::Identifier>("main"),
				makeVector< shader_precompiler::ast::nodes::VariableInitialization>()
			),
			std::make_unique< shader_precompiler::ast::nodes::CodeBlock>()
		)
	);

	ASSERT_EQ(*rightTree, *tree) << tree->toDebugString(0);
}

TEST(AstTests, PlusFunc) {
	auto tree = processAst("int plus(int one, int two){ return one+two; }");


	auto rightTree = new shader_precompiler::ast::nodes::CodeBlock();
	rightTree->expressions.push_back(
		std::make_unique<shader_precompiler::ast::nodes::Func>(
			std::make_unique<shader_precompiler::ast::nodes::FuncDeclaration>(
				std::make_unique< shader_precompiler::ast::nodes::Identifier>("int"),
				std::make_unique< shader_precompiler::ast::nodes::Identifier>("plus"),
				makeVector< shader_precompiler::ast::nodes::VariableInitialization>(
					std::make_unique< shader_precompiler::ast::nodes::VariableInitialization>(
						std::make_unique< shader_precompiler::ast::nodes::Identifier>("int"),
						std::make_unique< shader_precompiler::ast::nodes::Identifier>("one")
					),
					std::make_unique< shader_precompiler::ast::nodes::VariableInitialization>(
						std::make_unique< shader_precompiler::ast::nodes::Identifier>("int"),
						std::make_unique< shader_precompiler::ast::nodes::Identifier>("two")
					)
				)
			),
			std::make_unique< shader_precompiler::ast::nodes::CodeBlock>(
				makeVector<shader_precompiler::ast::nodes::Node>(
					std::make_unique<shader_precompiler::ast::nodes::Return>(
						std::make_unique<shader_precompiler::ast::nodes::Operator>(
							std::make_unique< shader_precompiler::ast::nodes::Identifier>("one"),
							"+",
							std::make_unique< shader_precompiler::ast::nodes::Identifier>("two")
						)
					)
				)
			)
		)
	);

	ASSERT_EQ(*rightTree, *tree) << tree->toDebugString(0);
}