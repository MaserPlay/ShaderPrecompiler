#include "ast.hpp"

#include "print_error.hpp"
#include "utils/unique_ptr_casts.hpp"

enum class ErrorCodes {
	UNEXPECTED_TOKEN
};

static void printError(ErrorCodes code, std::string text, std::size_t line, std::size_t column) {
	shader_precompiler::setError(shader_precompiler::Error{
		shader_precompiler::Error::Stage::AST, (std::size_t)code, text, line, column
		});
}

std::shared_ptr<shader_precompiler::ast::nodes::CodeBlock> shader_precompiler::ast::AstParser::createTree() {
	if (base != NULL) {
		return base;
	}
	base = std::make_shared<shader_precompiler::ast::nodes::CodeBlock>();

	while (auto first = from.peek()) {

		if (first->type == shader_precompiler::lexer::Token::Type::Identifier) {
			base->expressions.push_back(parseExpression());
		}
		else if (first->type == shader_precompiler::lexer::Token::Type::Symbol && first->text == ";") {
			from.get();
		}
		else {
			printError(ErrorCodes::UNEXPECTED_TOKEN, "Unexpected token: " + first->toDebugString(), first->line, first->column);
			from.get();
		}
	}

	return base;
}

bool isSingle(shader_precompiler::lexer::Token::Type t) {
	return t == shader_precompiler::lexer::Token::Type::Identifier ||
		t == shader_precompiler::lexer::Token::Type::Number ||
		t == shader_precompiler::lexer::Token::Type::String;
}

short precedence(const std::string& op)
{
	if (op == "*" || op == "/") return 4;
	if (op == "+" || op == "-") return 3;
	return 0;
}

std::unique_ptr<shader_precompiler::ast::nodes::Node> shader_precompiler::ast::AstParser::parseSingle() {
	auto token = from.peek();

	if (token && isSingle(token->type)) {
		from.get();

		if (token->type == shader_precompiler::lexer::Token::Type::Number) {
			return std::make_unique<
				shader_precompiler::ast::nodes::NumberExpr
			>(std::stoull(token->text));
		}
		return std::make_unique<
			shader_precompiler::ast::nodes::Identifier
		>(token->text);
	}
	return NULL;
}
std::unique_ptr<shader_precompiler::ast::nodes::Node> shader_precompiler::ast::AstParser::parseInitialization() {
	auto varInit = std::make_unique<shader_precompiler::ast::nodes::VariableInitialization>();
	varInit->type = static_unique_cast_ptr<shader_precompiler::ast::nodes::Identifier>(parseSingle());
	varInit->name = static_unique_cast_ptr<shader_precompiler::ast::nodes::Identifier>(parseSingle());

	return varInit;
}
std::unique_ptr<shader_precompiler::ast::nodes::Node> shader_precompiler::ast::AstParser::parseExpression(int minPrec) {

	auto first = from.peek();

	if (!first)
		return NULL;

	std::unique_ptr<shader_precompiler::ast::nodes::Node> left{};

	if (first->type == shader_precompiler::lexer::Token::Type::Identifier && 
		types.find(first->text) != end(types)) {
		left = parseInitialization();
	}
	else {
		left = parseSingle();
	}

	if (!left) return NULL;

	while (true)
	{
		auto op = from.peek();
		if (!op || op->type != shader_precompiler::lexer::Token::Type::Operator) break;

		int prec = precedence(op->text);
		if (prec < minPrec) break;

		from.get();

		auto right = parseExpression(prec + 2);

		auto operator_ = std::make_unique<shader_precompiler::ast::nodes::Operator>();


		operator_->op = op->text;
		operator_->left = std::move(left);
		operator_->right = std::move(right);

		left = std::move(operator_);
	}
	return left;
}