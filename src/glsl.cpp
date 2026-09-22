#include "to_glsl.hpp"

#include <charconv>

void shader_precompiler::GlslVisitor::generate() {
	out << "#version 330 core\n";
	for (auto& t : from)
	{
		t->accept(*this);
		if (dynamic_cast<shader_precompiler::ast::nodes::Func*>(t.get())) {

		}
		else {
			out << ";";
		}
	}
}
void shader_precompiler::GlslVisitor::visit(shader_precompiler::ast::nodes::CodeBlock& node) {

	out << "{";
	for (auto& expr : node.expressions)
	{
		expr->accept(*this);
		out << ";";
	}
	out << "}";
}
void shader_precompiler::GlslVisitor::visit(shader_precompiler::ast::nodes::Identifier& node) {
	out << node.name;
}
void shader_precompiler::GlslVisitor::visit(shader_precompiler::ast::nodes::Return& node) {
	out << "return ";
	
	if (node.value) {
		node.value->accept(*this);
	}
}
void shader_precompiler::GlslVisitor::visit(shader_precompiler::ast::nodes::Attribute& node) {

}
void shader_precompiler::GlslVisitor::visit(shader_precompiler::ast::nodes::VariableInitialization& node) {
	bool haveAttribute = false;
	if (shader_precompiler::ast::haveAttribute(node.attributes, "__glsl_in")) {
		if (haveAttribute) {
			printError(Error::Level::WARNING, Error::ErrorCodes::MULTIPLE_ATTRIBUTES, Error::makeStore(), node.location);
		}
		haveAttribute = true;
		out << "in ";
	} 
	if (shader_precompiler::ast::haveAttribute(node.attributes, "__glsl_out")) {
		if (haveAttribute) {
			printError(Error::Level::WARNING, Error::ErrorCodes::MULTIPLE_ATTRIBUTES, Error::makeStore(), node.location);
		}
		haveAttribute = true;
		out << "out ";
	}
	if (shader_precompiler::ast::haveAttribute(node.attributes, "__glsl_uniform")) {
		if (haveAttribute) {
			printError(Error::Level::WARNING, Error::ErrorCodes::MULTIPLE_ATTRIBUTES, Error::makeStore(), node.location);
		}
		haveAttribute = true;
		out << "uniform ";
	}
	if (auto l = shader_precompiler::ast::getFuncAttribute(node.attributes, "__glsl_layout_in"); l != NULL) {
		if (haveAttribute) {
			printError(Error::Level::WARNING, Error::ErrorCodes::MULTIPLE_ATTRIBUTES, Error::makeStore(), node.location);
		}
		haveAttribute = true;
		if (l->params.size() != 1) {
			printError(Error::Level::WARNING, Error::ErrorCodes::LAYOUT_IN_INVALID_PARAMS, Error::makeStore(), l->location);
		}

		out << "layout(location=";

		for (auto& p : l->params)
		{
			if (auto n = dynamic_cast<shader_precompiler::ast::nodes::NumberExpr*>(p.get()); n == NULL) {
				printError(Error::Level::WARNING, Error::ErrorCodes::LAYOUT_IN_INVALID_PARAMS, Error::makeStore(), p->location);
			}
			p->accept(*this);
		}
		out << ")in ";
	}
	node.type->accept(*this);
	out << " ";
	node.name->accept(*this);
}
void shader_precompiler::GlslVisitor::visit(shader_precompiler::ast::nodes::IfElse& node) {
	out << "if(";
	node.ifCondition->accept(*this);
	out << ")";

	node.thenBranch->accept(*this);
	if (node.elseBranch != NULL) {
		out << "else";

		node.elseBranch->accept(*this);
	}
}
static bool needBrackets(
	shader_precompiler::ast::nodes::Operator::Type parent,
	shader_precompiler::ast::nodes::Operator::Type child,
	bool rightChild
)
{
	int parentPrec;
	int childPrec;

	for (const auto& info : shader_precompiler::ast::operatorsInfo) {
		if (info.type == parent) {
			parentPrec = info.precedence;
		}
		else if (info.type == child) {
			childPrec = info.precedence;
		}
	}


	if (childPrec < parentPrec)
		return true;

	if (childPrec > parentPrec)
		return false;

	// Одинаковый precedence.

	if (!rightChild)
		return false;

	using Type = shader_precompiler::ast::nodes::Operator::Type;

	switch (parent)
	{
	case Type::SUBTRACT:
		// a - (b + c)
		// a - (b - c)
		return child == Type::ADD ||
			child == Type::SUBTRACT;

	case Type::DIVIDE:
		// a / (b * c)
		// a / (b / c)
		return child == Type::MULTIPLY ||
			child == Type::DIVIDE;

	default:
		return false;
	}
}

void shader_precompiler::GlslVisitor::visit(shader_precompiler::ast::nodes::Operator& node) {

	if (
		auto* childOp = dynamic_cast<shader_precompiler::ast::nodes::Operator*>(node.left.get());
		childOp && needBrackets(node.op, childOp->op, false)
		) {
		out << "(";
		node.left->accept(*this);
		out << ")";
	}
	else {
		node.left->accept(*this);
	}

	if (node.op == shader_precompiler::ast::nodes::Operator::Type::INDEX) {
		out << "[";
		node.right->accept(*this);
		out << "]";
	}
	else {
		out << shader_precompiler::ast::nodes::Operator::operatorTypeToString(node.op);

		if (
			auto* childOp = dynamic_cast<shader_precompiler::ast::nodes::Operator*>(node.right.get());
			childOp && needBrackets(node.op, childOp->op, true)
			) {
			out << "(";
			node.right->accept(*this);
			out << ")";
		}
		else {
			node.right->accept(*this);
		}

	}
}
void shader_precompiler::GlslVisitor::visit(shader_precompiler::ast::nodes::FuncDeclaration& node) {
	node.returnType->accept(*this);
	out << " ";
	node.name->accept(*this);
	out << "(";
	bool placeComma = false;
	for (auto& param : node.params)
	{
		if (placeComma) {
			out << ",";
		}
		param->type->accept(*this);
		out << " ";
		param->name->accept(*this);
		placeComma = true;
	}
	out << ")";
}
void shader_precompiler::GlslVisitor::visit(shader_precompiler::ast::nodes::FuncCall& node) {
	node.name->accept(*this);
	out << "(";
	bool placeComma = false;
	for (auto& param : node.params)
	{
		if (placeComma) {
			out << ",";
		}
		param->accept(*this);
		placeComma = true;
	}
	out << ")";
}
void shader_precompiler::GlslVisitor::visit(shader_precompiler::ast::nodes::Func& node) {

	node.declaration->accept(*this);

	node.code->accept(*this);
}
void shader_precompiler::GlslVisitor::visit(shader_precompiler::ast::nodes::NumberExpr& node) {

	char buf[32];

	auto [ptr, ec] = std::to_chars(
		buf,
		buf + sizeof(buf),
		node.value,
		std::chars_format::general);

	std::string value(buf, ptr);

	out << value;

	if (node.floating && 
		value.find('.') == std::string::npos) {
		out << ".";
	}
}