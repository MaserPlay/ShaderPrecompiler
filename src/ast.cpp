#include "ast.hpp"

#include <limits>

#include "utils/unique_ptr_casts.hpp"

std::vector<std::unique_ptr<shader_precompiler::ast::nodes::Node>> shader_precompiler::ast::AstParser::processTree() {
	std::vector<std::unique_ptr<shader_precompiler::ast::nodes::Node>> base {};

	while (auto first = from.peek()) {

		std::vector<std::unique_ptr<shader_precompiler::ast::nodes::Attribute>> attributes = {};

		while (auto attr = parseAttributes()) {
			attributes.push_back(std::move(attr));
		}

		if (auto decl = parseDeclaration(std::move(attributes)))
		{
			base.push_back(std::move(decl));
			continue;
		}

		if (auto stmt = parseExpression(parsePrimary()))
		{
			base.push_back(std::move(stmt));
			continue;
		}

		printError(shader_precompiler::Error::Level::WARNING, shader_precompiler::Error::ErrorCodes::UNEXPECTED_START_TOKEN, shader_precompiler::Error::makeStore(first->toDebugString()), *first);
		from.get();
		continue;
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
	block->location = first->location;

	while (auto next = from.peek()) {

		if (next->type == shader_precompiler::lexer::Token::Type::Symbol &&
			next->text == "}") {
			from.get();
			break;
		}

		if (auto re = parseIfElse())
		{
			block->expressions.push_back(std::move(re));
		}
		else if (auto re = parseReturn())
		{
			block->expressions.push_back(std::move(re));
		}
		else if (auto stmt = parseExpression(parsePrimary()))
		{
			block->expressions.push_back(std::move(stmt));
		}
		else if (auto decl = parseDeclaration())
		{
			block->expressions.push_back(std::move(decl));
		}
		else {
			printError(shader_precompiler::Error::Level::WARNING, shader_precompiler::Error::ErrorCodes::UNEXPECTED_START_TOKEN, shader_precompiler::Error::makeStore(next->toDebugString()), *next);
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
	std::unique_ptr<shader_precompiler::ast::nodes::Node> ret = NULL;

	if (token && isSingle(token->type)) {
		from.get();

		if (token->type == shader_precompiler::lexer::Token::Type::Number) {
			ret = std::make_unique<
				shader_precompiler::ast::nodes::NumberExpr
			>(std::stoull(token->text));
			ret->location = token->location;
		}
		else {
			ret = std::make_unique<
				shader_precompiler::ast::nodes::Identifier
			>(token->text);
			ret->location = token->location;
		}
	}
	return ret;
}
std::unique_ptr<shader_precompiler::ast::nodes::Node> shader_precompiler::ast::AstParser::parseDeclaration(std::vector<std::unique_ptr<shader_precompiler::ast::nodes::Attribute>> attributes) {

	auto firstToken = from.peek();

	if (!firstToken ||
		firstToken->type != shader_precompiler::lexer::Token::Type::Identifier) {
		return NULL;
	}

	auto first = parseSingle();

	auto secondToken = from.peek();

	if (!secondToken ||
		secondToken->type != shader_precompiler::lexer::Token::Type::Identifier) {
		if (secondToken) {
			printError(shader_precompiler::Error::Level::INFO, shader_precompiler::Error::ErrorCodes::TYPE_ALONE, shader_precompiler::Error::makeStore(firstToken->toDebugString()), *secondToken);
		}
		else {
			printError(shader_precompiler::Error::Level::INFO, shader_precompiler::Error::ErrorCodes::TYPE_ALONE, shader_precompiler::Error::makeStore(firstToken->toDebugString()), *firstToken);
		}
		return NULL;
	}

	auto second = parseSingle();

	auto thirdToken = from.peek();

	if (!thirdToken ||
		*thirdToken == shader_precompiler::lexer::Token(shader_precompiler::lexer::Token::Type::Symbol, ";")) {
		return parseVariableInitialization(std::move(first), std::move(second), std::move(attributes));
	}

	if (thirdToken->type == shader_precompiler::lexer::Token::Type::Symbol &&
		thirdToken->text == "(") {
		return parseFunction(std::move(first), std::move(second), std::move(attributes));
	}
	else {
		return parseExpression(parseVariableInitialization(std::move(first), std::move(second), std::move(attributes)));
	}
}
std::unique_ptr<shader_precompiler::ast::nodes::Node> shader_precompiler::ast::AstParser::parseFunction(std::unique_ptr<shader_precompiler::ast::nodes::Node> returnType, std::unique_ptr<shader_precompiler::ast::nodes::Node> name , std::vector<std::unique_ptr<shader_precompiler::ast::nodes::Attribute>> attributes) {
	auto funcDeclInit = std::make_unique<shader_precompiler::ast::nodes::FuncDeclaration>();
	funcDeclInit->location = returnType->location;
	funcDeclInit->returnType = static_unique_cast_ptr<shader_precompiler::ast::nodes::Identifier>(returnType);
	funcDeclInit->name = static_unique_cast_ptr<shader_precompiler::ast::nodes::Identifier>(name);
	funcDeclInit->attributes = std::move(attributes);

	auto nextToken = from.peek();

	if (!nextToken || *nextToken != shader_precompiler::lexer::Token(shader_precompiler::lexer::Token::Type::Symbol, "(")) {
		return funcDeclInit;
	}
	from.get();

	while (true) {
		auto firstToken = from.peek();

		if (firstToken && firstToken->type == shader_precompiler::lexer::Token::Type::Identifier) {
		}
		else {
			break;
		}

		auto first = parseSingle();

		auto secondToken = from.peek();

		if (secondToken && secondToken->type == shader_precompiler::lexer::Token::Type::Identifier) {
		}
		else {
			printError(shader_precompiler::Error::Level::INFO, shader_precompiler::Error::ErrorCodes::TYPE_ALONE, shader_precompiler::Error::makeStore(firstToken->toDebugString()), *firstToken);
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
			printError(shader_precompiler::Error::Level::ERROR, shader_precompiler::Error::ErrorCodes::NO_CLOSE_BRACKET_TOKEN, shader_precompiler::Error::makeStore(), *bracketToken);
		}
		else {
			printError(shader_precompiler::Error::Level::ERROR, shader_precompiler::Error::ErrorCodes::NO_CLOSE_BRACKET_TOKEN, shader_precompiler::Error::makeStore(), *nextToken);
		}
	}
	from.get();

	auto code = parseCodeBlock();

	if (!code) {
		return funcDeclInit;
	}

	auto funcInit = std::make_unique<shader_precompiler::ast::nodes::Func>();

	funcInit->declaration = std::move(funcDeclInit);
	funcInit->location = code->location;
	funcInit->code = std::move(code);

	return funcInit;
}
std::unique_ptr<shader_precompiler::ast::nodes::VariableInitialization> shader_precompiler::ast::AstParser::parseVariableInitialization(std::unique_ptr<shader_precompiler::ast::nodes::Node> type, std::unique_ptr<shader_precompiler::ast::nodes::Node> name, std::vector<std::unique_ptr<shader_precompiler::ast::nodes::Attribute>> attributes) {
	auto varInit = std::make_unique<shader_precompiler::ast::nodes::VariableInitialization>();
	varInit->location = type->location;
	varInit->type = static_unique_cast_ptr<shader_precompiler::ast::nodes::Identifier>(type);
	varInit->name = static_unique_cast_ptr<shader_precompiler::ast::nodes::Identifier>(name);
	varInit->attributes = std::move(attributes);

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
std::unique_ptr<shader_precompiler::ast::nodes::Node> shader_precompiler::ast::AstParser::parseIfElse() {

	auto ifToken = from.peek();
	if (!(ifToken &&
		ifToken->type == shader_precompiler::lexer::Token::Type::Identifier &&
		ifToken->text == "if")) {
		return NULL;
	}
	from.get();

	auto node = std::make_unique<shader_precompiler::ast::nodes::IfElse>();

	node->location = ifToken->location;
	node->ifCondition = parseBrackets();
	node->thenBranch = parseCodeBlock();

	auto elseToken = from.peek();
	if (!(elseToken &&
		elseToken->type == shader_precompiler::lexer::Token::Type::Identifier &&
		elseToken->text == "else")) {
		return node;
	}
	from.get();

	auto elseIfToken = from.peek();
	if (elseIfToken &&
		elseIfToken->type == shader_precompiler::lexer::Token::Type::Identifier &&
		elseIfToken->text == "if") {
		node->elseBranch = parseIfElse();
	}
	else {
		node->elseBranch = parseCodeBlock();
	}

	return node;
}
std::unique_ptr<shader_precompiler::ast::nodes::Attribute> shader_precompiler::ast::AstParser::parseAttributes() {
	auto open = from.peek();
	if (!(open &&
		open->type == shader_precompiler::lexer::Token::Type::Symbol &&
		open->text == "[")) {
		return NULL;
	}
	from.get();

	open = from.peek();
	if (!(open &&
		open->type == shader_precompiler::lexer::Token::Type::Symbol &&
		open->text == "[")) {
		return NULL;
	}
	from.get();

	auto node = std::make_unique<shader_precompiler::ast::nodes::Attribute>();

	node->location = open->location;
	auto value = parsePrimary();

	if (auto idValue = dynamic_unique_cast_ptr<shader_precompiler::ast::nodes::Identifier>(value); idValue != NULL) {
		node->value = parseFunctionCall(idValue);
		if (node->value == NULL) {
			node->value = std::move(idValue);
		}
	}
	else {
		node->value = std::move(value);
	}

	auto close = from.peek();
	if (!(close &&
		close->type == shader_precompiler::lexer::Token::Type::Symbol &&
		close->text == "]")) {
		if (close) {
			printError(shader_precompiler::Error::Level::ERROR, shader_precompiler::Error::ErrorCodes::NO_CLOSE_ATTRIBUTE_TOKEN, shader_precompiler::Error::makeStore(), *close);
		}
		else {
			printError(shader_precompiler::Error::Level::ERROR, shader_precompiler::Error::ErrorCodes::NO_CLOSE_ATTRIBUTE_TOKEN, shader_precompiler::Error::makeStore(), *open);
		}
		return node;
	}
	from.get();

	close = from.peek();
	if (!(close &&
		close->type == shader_precompiler::lexer::Token::Type::Symbol &&
		close->text == "]")) {
		if (close) {
			printError(shader_precompiler::Error::Level::ERROR, shader_precompiler::Error::ErrorCodes::NO_CLOSE_ATTRIBUTE_TOKEN, shader_precompiler::Error::makeStore(), *close);
		}
		else {
			printError(shader_precompiler::Error::Level::ERROR, shader_precompiler::Error::ErrorCodes::NO_CLOSE_ATTRIBUTE_TOKEN, shader_precompiler::Error::makeStore(), *open);
		}
		return node;
	}
	from.get();

	return node;
}
std::unique_ptr<shader_precompiler::ast::nodes::Node> shader_precompiler::ast::AstParser::parsePrimary() {
	if (auto curr = parseBrackets())
	{
		return (std::move(curr));
	}

	if (auto curr = parseSingle())
	{
		if (auto id = dynamic_unique_cast_ptr<shader_precompiler::ast::nodes::Identifier>(std::move(curr)); id != NULL) {
			auto ret = parseFunctionCall(id);
			if (ret != NULL) {
				return ret;
			}
			auto next = from.peek();
			if (next && next->type == shader_precompiler::lexer::Token::Type::Identifier) {
				return parseVariableInitialization(std::move(id), parseSingle());
			}
			return id;
		}
		else {
			return std::move(curr);
		}
	}
	return NULL;
}
std::unique_ptr<shader_precompiler::ast::nodes::Node> shader_precompiler::ast::AstParser::parseFunctionCall(std::unique_ptr<shader_precompiler::ast::nodes::Identifier>& name) {

	if (!name) return NULL;

	auto nextToken = from.peek();

	auto func = std::make_unique<shader_precompiler::ast::nodes::FuncCall>();

	if (!nextToken || *nextToken != shader_precompiler::lexer::Token(shader_precompiler::lexer::Token::Type::Symbol, "(")) {
		return NULL;
	}
	from.get();
	func->location = name->location;
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
			printError(shader_precompiler::Error::Level::ERROR, shader_precompiler::Error::ErrorCodes::NO_CLOSE_BRACKET_TOKEN, shader_precompiler::Error::makeStore(), *bracketToken);
		}
		else {
			printError(shader_precompiler::Error::Level::ERROR, shader_precompiler::Error::ErrorCodes::NO_CLOSE_BRACKET_TOKEN, shader_precompiler::Error::makeStore(), *nextToken);
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
		auto op1 = from.peek();
		if (!op1) {
			break;
		}
		else if (op1->type != shader_precompiler::lexer::Token::Type::Operator) {
			break;
		}

		left->location = op1->location;

		auto op = op1->text;

		short prec = precedence(op);

		if (prec < minPrec) break;

		from.get();

		auto right = parseExpression(parsePrimary(), prec + 2);

		auto operator_ = std::make_unique<shader_precompiler::ast::nodes::Operator>();


		operator_->op = op;
		operator_->left = std::move(left);
		operator_->right = std::move(right);

		left = std::move(operator_);
	}
	return left;
}