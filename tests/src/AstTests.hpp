#include "../common.hpp"

#include "precompiler.hpp"
#include "diagnostic_reporters.hpp"
#include "ast.hpp"

static auto processAst(std::string base) {
	shader_precompiler::PrintDiagnostic printDia(shader_precompiler::locales::Locales::ENGLISH);
	std::istringstream iss(base);

	shader_precompiler::lexer::LexerStream tokenStream(iss, printDia);

	shader_precompiler::precompiler::PrecompilerLexerStream afterPreprocessor(tokenStream, printDia, emptyFileFactory, shader_precompiler::precompiler::Context{});

	shader_precompiler::ast::AstParser ast(afterPreprocessor, printDia);

	return ast.processTree();
}
template<class T, class... Args>
std::vector<std::unique_ptr<T>> makeVector(Args&&... args) {
	std::vector<std::unique_ptr<T>> vec;
	(vec.push_back(std::forward<Args>(args)), ...);
	return vec;
}

TEST(AstTests, SimplyExpression) {
	auto tree = processAst("int a = 2+2;");

	auto rightTree = makeVector<shader_precompiler::ast::nodes::Node>(
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

	ASSERT_TRUE_AST(rightTree, tree);
}

TEST(AstTests, RightOperationOrderExpression) {
	auto tree = processAst("int a = 2+(2*2);");

	auto rightTree = makeVector<shader_precompiler::ast::nodes::Node>(
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

	ASSERT_TRUE_AST(rightTree, tree);
}

TEST(AstTests, EmptyFunc) {
	auto tree = processAst("void main();void main(){}");


	auto rightTree = makeVector<shader_precompiler::ast::nodes::Node>(
		std::make_unique<shader_precompiler::ast::nodes::FuncDeclaration>(
			std::make_unique< shader_precompiler::ast::nodes::Identifier>("void"),
			std::make_unique< shader_precompiler::ast::nodes::Identifier>("main"),
			makeVector< shader_precompiler::ast::nodes::VariableInitialization>()
		)
		, std::make_unique<shader_precompiler::ast::nodes::Func>(
			std::make_unique<shader_precompiler::ast::nodes::FuncDeclaration>(
				std::make_unique< shader_precompiler::ast::nodes::Identifier>("void"),
				std::make_unique< shader_precompiler::ast::nodes::Identifier>("main"),
				makeVector< shader_precompiler::ast::nodes::VariableInitialization>()
			),
			std::make_unique< shader_precompiler::ast::nodes::CodeBlock>()
		)
	);

	ASSERT_TRUE_AST(rightTree, tree);
}

TEST(AstTests, PlusFunc) {
	auto tree = processAst("int plus(int one, int two){ return one+two; }void main(){int p = plus(2,3);plus(1,2);}");


	auto rightTree = makeVector<shader_precompiler::ast::nodes::Node>(
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
		,
		std::make_unique<shader_precompiler::ast::nodes::Func>(
			std::make_unique<shader_precompiler::ast::nodes::FuncDeclaration>(
				std::make_unique< shader_precompiler::ast::nodes::Identifier>("void"),
				std::make_unique< shader_precompiler::ast::nodes::Identifier>("main"),
				makeVector< shader_precompiler::ast::nodes::VariableInitialization>()
			),
			std::make_unique< shader_precompiler::ast::nodes::CodeBlock>(
				makeVector<shader_precompiler::ast::nodes::Node>(
					std::make_unique<shader_precompiler::ast::nodes::Operator>(
						std::make_unique< shader_precompiler::ast::nodes::VariableInitialization>(
							std::make_unique< shader_precompiler::ast::nodes::Identifier>("int"),
							std::make_unique< shader_precompiler::ast::nodes::Identifier>("p")
						),
						"=",
						std::make_unique<shader_precompiler::ast::nodes::FuncCall>(
							std::make_unique< shader_precompiler::ast::nodes::Identifier>("plus"),
							makeVector< shader_precompiler::ast::nodes::Node>(
								std::make_unique< shader_precompiler::ast::nodes::NumberExpr>(2),
								std::make_unique< shader_precompiler::ast::nodes::NumberExpr>(3)
							)
						)
					),
					std::make_unique<shader_precompiler::ast::nodes::FuncCall>(
						std::make_unique< shader_precompiler::ast::nodes::Identifier>("plus"),
						makeVector< shader_precompiler::ast::nodes::Node>(
							std::make_unique< shader_precompiler::ast::nodes::NumberExpr>(1),
							std::make_unique< shader_precompiler::ast::nodes::NumberExpr>(2)
						)
					)
				)
			)
		)
	);

	ASSERT_TRUE_AST(rightTree, tree);
}

TEST(AstTests, IfElseLadder) {
	auto tree = processAst(R"(
void ladder(int number1, int number2) {
    //checks if the two integers are equal.
    if(number1 == number2) {
        equals(number1,number2);
    }

    //checks if number1 is greater than number2.
    else if (number1 > number2) {
        greater(number1, number2);
    }

    //checks if both test expressions are false
    else {
        less(number1, number2);
    }
})");


	auto rightTree = makeVector<shader_precompiler::ast::nodes::Node>(
		std::make_unique<shader_precompiler::ast::nodes::Func>(
			std::make_unique<shader_precompiler::ast::nodes::FuncDeclaration>(
				std::make_unique< shader_precompiler::ast::nodes::Identifier>("void"),
				std::make_unique< shader_precompiler::ast::nodes::Identifier>("ladder"),
				makeVector< shader_precompiler::ast::nodes::VariableInitialization>(
					std::make_unique< shader_precompiler::ast::nodes::VariableInitialization>(
						std::make_unique< shader_precompiler::ast::nodes::Identifier>("int"),
						std::make_unique< shader_precompiler::ast::nodes::Identifier>("number1")
					),
					std::make_unique< shader_precompiler::ast::nodes::VariableInitialization>(
						std::make_unique< shader_precompiler::ast::nodes::Identifier>("int"),
						std::make_unique< shader_precompiler::ast::nodes::Identifier>("number2")
					)
				)
			),
			std::make_unique< shader_precompiler::ast::nodes::CodeBlock>(
				makeVector<shader_precompiler::ast::nodes::Node>(
					std::make_unique<shader_precompiler::ast::nodes::IfElse>(
						std::make_unique<shader_precompiler::ast::nodes::Operator>(
							std::make_unique< shader_precompiler::ast::nodes::Identifier>("number1"),
							"==",
							std::make_unique< shader_precompiler::ast::nodes::Identifier>("number2")
						),
						std::make_unique< shader_precompiler::ast::nodes::CodeBlock>(
							makeVector<shader_precompiler::ast::nodes::Node>(
								std::make_unique<shader_precompiler::ast::nodes::FuncCall>(
									std::make_unique< shader_precompiler::ast::nodes::Identifier>("equals"),
									makeVector< shader_precompiler::ast::nodes::Node>(
										std::make_unique< shader_precompiler::ast::nodes::Identifier>("number1"),
										std::make_unique< shader_precompiler::ast::nodes::Identifier>("number2")
									)
								)
							)
						),
						std::make_unique<shader_precompiler::ast::nodes::IfElse>(
							std::make_unique<shader_precompiler::ast::nodes::Operator>(
								std::make_unique< shader_precompiler::ast::nodes::Identifier>("number1"),
								">",
								std::make_unique< shader_precompiler::ast::nodes::Identifier>("number2")
							),
							std::make_unique< shader_precompiler::ast::nodes::CodeBlock>(
								makeVector<shader_precompiler::ast::nodes::Node>(
									std::make_unique<shader_precompiler::ast::nodes::FuncCall>(
										std::make_unique< shader_precompiler::ast::nodes::Identifier>("greater"),
										makeVector< shader_precompiler::ast::nodes::Node>(
											std::make_unique< shader_precompiler::ast::nodes::Identifier>("number1"),
											std::make_unique< shader_precompiler::ast::nodes::Identifier>("number2")
										)
									)
								)
							),
							std::make_unique< shader_precompiler::ast::nodes::CodeBlock>(
								makeVector<shader_precompiler::ast::nodes::Node>(
									std::make_unique<shader_precompiler::ast::nodes::FuncCall>(
										std::make_unique< shader_precompiler::ast::nodes::Identifier>("less"),
										makeVector< shader_precompiler::ast::nodes::Node>(
											std::make_unique< shader_precompiler::ast::nodes::Identifier>("number1"),
											std::make_unique< shader_precompiler::ast::nodes::Identifier>("number2")
										)
									)
								)
							)
						)
					)
				)
			)
		)
	);

	ASSERT_TRUE_AST(rightTree, tree);
}

TEST(AstTests, Atrributes) {
	auto tree = processAst("[[func(param)]] int value; [[param]] void main(){}");


	auto rightTree = makeVector<shader_precompiler::ast::nodes::Node>(
		std::make_unique<shader_precompiler::ast::nodes::VariableInitialization>(
			std::make_unique< shader_precompiler::ast::nodes::Identifier>("int"),
			std::make_unique< shader_precompiler::ast::nodes::Identifier>("value"),
			makeVector<shader_precompiler::ast::nodes::Attribute>(
				std::make_unique< shader_precompiler::ast::nodes::Attribute>(
					std::make_unique<shader_precompiler::ast::nodes::FuncCall>(
						std::make_unique< shader_precompiler::ast::nodes::Identifier>("func"),
						makeVector< shader_precompiler::ast::nodes::Node>(
							std::make_unique< shader_precompiler::ast::nodes::Identifier>("param")
						)
					)
				)
			)
		)
		, std::make_unique<shader_precompiler::ast::nodes::Func>(
			std::make_unique<shader_precompiler::ast::nodes::FuncDeclaration>(
				std::make_unique< shader_precompiler::ast::nodes::Identifier>("void"),
				std::make_unique< shader_precompiler::ast::nodes::Identifier>("main"),
				makeVector< shader_precompiler::ast::nodes::VariableInitialization>(),
				makeVector<shader_precompiler::ast::nodes::Attribute>(
					std::make_unique< shader_precompiler::ast::nodes::Attribute>(
						std::make_unique< shader_precompiler::ast::nodes::Identifier>("param")
					)
				)
			),
			std::make_unique< shader_precompiler::ast::nodes::CodeBlock>()
		)
	);

	ASSERT_TRUE_AST(rightTree, tree);
}