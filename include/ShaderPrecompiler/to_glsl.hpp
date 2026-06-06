#pragma once

#include "ast.hpp"

namespace shader_precompiler {
	/// <summary>
	/// [[__glsl_in]] [[__glsl_out]] [[__glsl_uniform]] [[__glsl_layout_in(0)]]
	/// </summary>
	class GlslVisitor : public shader_precompiler::ast::VisitorBase {
		shader_precompiler::ast::TreeResult from;
		IDiagnosticReporter& reporter;
		std::ostream& out;

		PRINT_ERROR_DEFINE(shader_precompiler::Error::Stage::GLSL)

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
		GlslVisitor(shader_precompiler::ast::TreeResult from, IDiagnosticReporter& reporter, std::ostream& to) : from(std::move(from)), reporter(reporter), out(to) {};
		GlslVisitor(shader_precompiler::ast::BaseAstProcessor& from, IDiagnosticReporter& reporter, std::ostream& to) : GlslVisitor(std::move(from.processTree()), reporter, to) {};
		void generate();
	};
}