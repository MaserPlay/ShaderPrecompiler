#include "ast.hpp"

#include <limits>

#include "print_error.hpp"
#include "utils/unique_ptr_casts.hpp"

enum class ErrorCodes {
	UNEXPECTED_TOKEN,
	TYPE_ALONE,
	FUNCTION_PARAM_WHERE_IS_TYPE,
	WHERE_IS
};

static void printError(ErrorCodes code, std::string text, shader_precompiler::lexer::Token token) {
	shader_precompiler::setError(shader_precompiler::Error{
		shader_precompiler::Error::Stage::AST, (std::size_t)code, text, token.line, token.column
		});
}

std::shared_ptr<shader_precompiler::ast::nodes::CodeBlock> shader_precompiler::ast::AstParser::createTree() {
	if (base != NULL) {
		return base;
	}
	base = std::make_shared<shader_precompiler::ast::nodes::CodeBlock>();

	while (auto first = from.peek()) {

		if (auto decl = parseDeclaration())
		{
			base->expressions.push_back(std::move(decl));
		}
		else if (auto stmt = parseExpression(parsePrimary()))
		{
			base->expressions.push_back(std::move(stmt));
		}
		else
		{
			printError(ErrorCodes::UNEXPECTED_TOKEN, "Unexpected token " + first->toDebugString(), *from.get());
		}
	}

	return base;
}

static bool isSingle(shader_precompiler::lexer::Token::Type t) {
	return t == shader_precompiler::lexer::Token::Type::Identifier ||
		t == shader_precompiler::lexer::Token::Type::Number ||
		t == shader_precompiler::lexer::Token::Type::String;
}

static short precedence(const std::string& op)
{
	if (op == "*" || op == "/") return 4;
	if (op == "+" || op == "-") return 3;
	return 0;
}
std::unique_ptr<shader_precompiler::ast::nodes::CodeBlock> shader_precompiler::ast::AstParser::parseCodeBlock() {

	auto first = from.peek();
	if (!(first && (first->type == shader_precompiler::lexer::Token::Type::Symbol &&
		first->text == "{"))) {
		return NULL;
	}
	from.get();

	auto block = std::make_unique< shader_precompiler::ast::nodes::CodeBlock>();

	while (auto next = from.peek()) {

		if (next->type == shader_precompiler::lexer::Token::Type::Symbol &&
			next->text == "}") {
			from.get();
			break;
		}

		if (auto re = parseReturn())
		{
			block->expressions.push_back(std::move(re));
		}
		else if (auto decl = parseDeclaration())
		{
			block->expressions.push_back(std::move(decl));
		}
		else if (auto stmt = parseExpression(parsePrimary()))
		{
			block->expressions.push_back(std::move(stmt));
		}
		else {
			printError(ErrorCodes::UNEXPECTED_TOKEN, "Unexpected token: " + next->toDebugString(), *next);
			from.get();
		}
	}

	return block;
}
std::unique_ptr<shader_precompiler::ast::nodes::Return> shader_precompiler::ast::AstParser::parseReturn() {
	auto token = from.peek();
	if (!(token &&
		token->type == shader_precompiler::lexer::Token::Type::Identifier &&
		token->text == "return")) {
		return NULL;
	}
	from.get();
	
	auto return_ = std::make_unique<shader_precompiler::ast::nodes::Return>();

	return_->value = parseExpression(parsePrimary());

	return (return_);
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
std::unique_ptr<shader_precompiler::ast::nodes::Node> shader_precompiler::ast::AstParser::parseDeclaration() {
	
	auto firstToken = from.peek();

	if (!firstToken ||
		firstToken->type != shader_precompiler::lexer::Token::Type::Identifier ||
		!isType(firstToken->text)) {
		return NULL;
	}

	auto first = parseSingle();

	auto secondToken = from.peek();

	if (!secondToken || 
		secondToken->type != shader_precompiler::lexer::Token::Type::Identifier ||
		isType(secondToken->text) ) { // TODO: PrintError
		if (secondToken) {
			printError(ErrorCodes::TYPE_ALONE, "Type " + firstToken->toDebugString() + " alone. Next token: " + secondToken->toDebugString(), *secondToken);
		}
		else {
			printError(ErrorCodes::TYPE_ALONE, "Type " + firstToken->toDebugString() + " alone. Next token: EMPTY", *firstToken);
		}
		return NULL;
	}

	auto second = parseSingle();

	auto thirdToken = from.peek();

	if (!thirdToken ||
		*thirdToken == shader_precompiler::lexer::Token(shader_precompiler::lexer::Token::Type::Symbol, ";")) {
		return parseVariableInitialization(std::move(first), std::move(second));
	}

	if (thirdToken->type == shader_precompiler::lexer::Token::Type::Symbol &&
		thirdToken->text == "(") {
		return parseFunction(std::move(first), std::move(second));
	}
	else {
		return parseExpression(parseVariableInitialization(std::move(first), std::move(second)));
	}
}
std::unique_ptr<shader_precompiler::ast::nodes::Node> shader_precompiler::ast::AstParser::parseFunction(std::unique_ptr<shader_precompiler::ast::nodes::Node> returnType, std::unique_ptr<shader_precompiler::ast::nodes::Node> name) {
	auto funcDeclInit = std::make_unique<shader_precompiler::ast::nodes::FuncDeclaration>();
	funcDeclInit->returnType = static_unique_cast_ptr<shader_precompiler::ast::nodes::Identifier>(returnType);
	funcDeclInit->name = static_unique_cast_ptr<shader_precompiler::ast::nodes::Identifier>(name);

	auto nextToken = from.peek();

	if (!nextToken || *nextToken != shader_precompiler::lexer::Token(shader_precompiler::lexer::Token::Type::Symbol, "(")) {
		return funcDeclInit;
	}
	from.get();

	while (true) {
		auto firstToken = from.peek();

		if (firstToken && firstToken->type == shader_precompiler::lexer::Token::Type::Identifier &&
			isType(firstToken->text)) {
		}
		else {
			break;
		}

		auto first = parseSingle();

		auto secondToken = from.peek();

		if (secondToken && secondToken->type == shader_precompiler::lexer::Token::Type::Identifier &&
			!isType(secondToken->text)) {
		}
		else {
			printError(ErrorCodes::TYPE_ALONE, "Type " + firstToken->toDebugString() + " alone.", *firstToken);
			return funcDeclInit;
		}

		auto second = parseSingle();

		funcDeclInit->params.push_back(parseVariableInitialization(std::move(first), std::move(second)));

		auto commaToken = from.peek();
		if (!(commaToken && commaToken->type == shader_precompiler::lexer::Token::Type::Symbol &&
			commaToken->text == ",")) {
			break;
		}
		from.get();
	}

	auto bracketToken = from.peek();
	if (!bracketToken || *bracketToken != shader_precompiler::lexer::Token(shader_precompiler::lexer::Token::Type::Symbol, ")")) {
		if (bracketToken) {
			printError(ErrorCodes::WHERE_IS, "WHERE IS ) Next token: " + bracketToken->toDebugString(), *bracketToken);
		}
		else {
			printError(ErrorCodes::WHERE_IS, "WHERE IS ) Next token: EMPTY", *nextToken);
		}
	}
	from.get();

	auto code = parseCodeBlock();

	if (!code) {
		return funcDeclInit;
	}

	auto funcInit = std::make_unique<shader_precompiler::ast::nodes::Func>();

	funcInit->declaration = std::move(funcDeclInit);
	funcInit->code = std::move(code);

	return funcInit;
}
std::unique_ptr<shader_precompiler::ast::nodes::VariableInitialization> shader_precompiler::ast::AstParser::parseVariableInitialization(std::unique_ptr<shader_precompiler::ast::nodes::Node> type, std::unique_ptr<shader_precompiler::ast::nodes::Node> name) {
	auto varInit = std::make_unique<shader_precompiler::ast::nodes::VariableInitialization>();
	varInit->type = static_unique_cast_ptr<shader_precompiler::ast::nodes::Identifier>(type);
	varInit->name = static_unique_cast_ptr<shader_precompiler::ast::nodes::Identifier>(name);

	return varInit;
}
std::unique_ptr<shader_precompiler::ast::nodes::Node> shader_precompiler::ast::AstParser::parseBrackets() {
	
	auto open = from.peek();

	if (!(open &&
		open->type == shader_precompiler::lexer::Token::Type::Symbol &&
		open->text == "(")) {
		return NULL;
	}
	from.get();

	auto expr = parseExpression(parsePrimary());

	auto close = from.peek();

	if (!(close &&
		close->type == shader_precompiler::lexer::Token::Type::Symbol &&
		close->text == ")")) {
	}
	else {
		from.get();
	}

	return expr;
}
std::unique_ptr<shader_precompiler::ast::nodes::Node> shader_precompiler::ast::AstParser::parsePrimary() {
	if (auto curr = parseBrackets())
	{
		return (std::move(curr));
	}

	if (auto curr = parseSingle())
	{
		if (auto id = dynamic_unique_cast_ptr<shader_precompiler::ast::nodes::Identifier>(std::move(curr)); id != NULL) {
			return parseFunctionCall(std::move(id));
		}
		else {
			return std::move(curr);
		}
	}
	return NULL;
}
std::unique_ptr<shader_precompiler::ast::nodes::Node> shader_precompiler::ast::AstParser::parseFunctionCall(std::unique_ptr<shader_precompiler::ast::nodes::Identifier> name) {

	if (!name) return NULL;

	auto nextToken = from.peek();

	auto func = std::make_unique<shader_precompiler::ast::nodes::FuncCall>();

	if (!nextToken || *nextToken != shader_precompiler::lexer::Token(shader_precompiler::lexer::Token::Type::Symbol, "(")) {
		return (name);
	}
	from.get();
	func->name = std::move(name);

	while (true) {
		auto firstToken = from.peek();

		if (firstToken) {
		}
		else {
			break;
		}

		auto first = parseSingle();

		func->params.push_back(std::move(first));

		auto commaToken = from.peek();
		if (!(commaToken && commaToken->type == shader_precompiler::lexer::Token::Type::Symbol &&
			commaToken->text == ",")) {
			break;
		}
		from.get();
	}

	auto bracketToken = from.peek();
	if (!bracketToken || *bracketToken != shader_precompiler::lexer::Token(shader_precompiler::lexer::Token::Type::Symbol, ")")) {
		if (bracketToken) {
			printError(ErrorCodes::WHERE_IS, "WHERE IS ) Next token: " + bracketToken->toDebugString(), *bracketToken);
		}
		else {
			printError(ErrorCodes::WHERE_IS, "WHERE IS ) Next token: EMPTY", *nextToken);
		}
	}
	from.get();

	return func;
}
std::unique_ptr<shader_precompiler::ast::nodes::Node> shader_precompiler::ast::AstParser::parseExpression(std::unique_ptr<shader_precompiler::ast::nodes::Node> left_, int minPrec) {

	if (!left_) return NULL;

	std::unique_ptr<shader_precompiler::ast::nodes::Node> left = std::move(left_);

	while (true)
	{
		auto op = from.peek();
		if (!op) {
			break;
		}
		else if (op->type != shader_precompiler::lexer::Token::Type::Operator) {
			break;
		}

		short prec = precedence(op->text);

		if (prec < minPrec) break;

		from.get();

		auto right = parseExpression(parsePrimary(), prec + 2);

		auto operator_ = std::make_unique<shader_precompiler::ast::nodes::Operator>();


		operator_->op = op->text;
		operator_->left = std::move(left);
		operator_->right = std::move(right);

		left = std::move(operator_);
	}
	return left;
}