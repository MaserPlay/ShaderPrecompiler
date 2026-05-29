#pragma once

#include <vector>
#include <string>

#include "ast.hpp"

namespace shader_precompiler {
	class SemanticVisitor : public shader_precompiler::ast::VisitorBase, public ast::BaseAstProcessor {
	public:
		struct Variable {
			std::string type{};
			std::string name{};
			Variable() = default;
			Variable(std::string type, std::string name) : type(type), name(name) {}
		};
		struct Func {
			std::string returnType{};
			std::string name{};
			std::vector<Variable> params{};
			Func() = default;
			Func(std::string returnType, std::string name, std::vector<Variable> params) : returnType(returnType), name(name), params(params) {}
		};
	private:
		shader_precompiler::ast::BaseAstProcessor& from;
		IDiagnosticReporter& reporter;
		std::vector<std::string> types{"int", "void", "bool", "float"};
		std::vector<Func> functions{};
		std::vector<Variable>* currentVariables{};

		PRINT_ERROR_DEFINE(shader_precompiler::Error::Stage::SEMANTIC_AMALIZITER)


		inline bool isType(std::string s) {
			for (auto& var : types)
			{
				if (var == s) {
					return true;
				}
			}
			return false;
		}

		inline bool isFunctionName(std::string s) {
			for (auto& var : functions)
			{
				if (var.name == s) {
					return true;
				}
			}
			return false;
		}

		inline bool isVariableName(std::string s) {
			for (auto& var : *currentVariables)
			{
				if (var.name == s) {
					return true;
				}
			}
			return false;
		}

		inline auto addVariable(Variable& v) {
			(*currentVariables).push_back(v);
		}

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
		SemanticVisitor(shader_precompiler::ast::BaseAstProcessor& from, IDiagnosticReporter& reporter) : from(from), reporter(reporter) {};
		std::vector<std::unique_ptr<shader_precompiler::ast::nodes::Node>> processTree() override;

		template<typename... Args>
		inline void addTypes(Args&&... args) {
			static_assert(
				((std::is_convertible_v<Args, std::string_view>) && ...),
				"All arguments must be string-like"
				);

			(types.push_back(std::string(args)), ...);
		}

		template<typename... Args>
		inline auto addFunctions(Args&&... args) {
			static_assert(
				((std::is_convertible_v<Args, Func>) && ...),
				"All arguments must be string-like"
				);

			(functions.push_back(args), ...);
		}
	};
}