#pragma once

#include "ast.hpp"
#include <vector>

namespace shader_precompiler::visitors {

	class MinimazerVisitor : public shader_precompiler::ast::VisitorBase, public shader_precompiler::ast::BaseAstProcessor {
		std::vector<std::string>* miniTable;

		shader_precompiler::ast::BaseAstProcessor& from;

		IDiagnosticReporter& reporter;


		PRINT_ERROR_DEFINE(shader_precompiler::Error::Stage::MINIMAZER)

		void visit(shader_precompiler::ast::nodes::CodeBlock& node) override;
		void visit(shader_precompiler::ast::nodes::Identifier& node) override;
		void visit(shader_precompiler::ast::nodes::Return& node) override;
		void visit(shader_precompiler::ast::nodes::Attribute& node) override;
		void visit(shader_precompiler::ast::nodes::VariableInitialization& node) override;
		void visit(shader_precompiler::ast::nodes::IfElse& node) override;
		void visit(shader_precompiler::ast::nodes::Operator& node) override;
		void visit(shader_precompiler::ast::nodes::FuncDeclaration& node) override;
		void visit(shader_precompiler::ast::nodes::FuncCall& node) override;
		void visit(shader_precompiler::ast::nodes::Func& node) override;
		void visit(shader_precompiler::ast::nodes::NumberExpr& node) override;
	public:
		MinimazerVisitor(shader_precompiler::ast::BaseAstProcessor& from, IDiagnosticReporter& reporter) : from(from), miniTable(NULL), reporter(reporter) {};
		std::shared_ptr<shader_precompiler::ast::nodes::CodeBlock> processTree() override;
	};
};