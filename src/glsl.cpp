#include "to_glsl.hpp"


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
	node.value->accept(*this);
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
void shader_precompiler::GlslVisitor::visit(shader_precompiler::ast::nodes::Operator& node) {

	node.left->accept(*this);
	out << node.op;
	node.right->accept(*this);
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
	out << std::to_string(node.value);
}